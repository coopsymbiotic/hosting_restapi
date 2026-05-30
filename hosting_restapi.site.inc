<?php

/**
 * @file
 *   Hosting REST API functions, and Drupal hooks.
 */

/**
 * Implements the 'site' API.
 */
function hosting_restapi_site() {
  hosting_restapi_cors_headers();

  $method = strtolower($_SERVER['REQUEST_METHOD']);
  $f = 'hosting_restapi_site_' . $method;

  try {
    if (function_exists($f)) {
      $result = $f();
    }
    else {
      $result = array(
        'status' => 'error',
        'message' => 'Unknown method for site: ' . $_SERVER['REQUEST_METHOD'],
      );
    }
  }
  catch (Exception $e) {
    $result = array(
      'status' => 'error',
      'message' => $e->getMessage(),
    );
  }

  echo json_encode($result);
  drupal_exit();
}

/**
 * Implements the 'site GET' API (get info/status).
 */
function hosting_restapi_site_get() {
  // FIXME: we want the user to have live updates on the status of the site,
  // so either we grant some temporary access, or we grant to anons.

  $access = hosting_restapi_check_access($_GET['key'], $_GET['secret']);
  if (!$access) {
    throw new Exception(t('Access denied.'));
  }

  $url = $_GET['url'];
  $invoice_id = (isset($_GET['invoice']) ? $_GET['invoice'] : NULL);

  if (! $url) {
    throw new Exception(t('The "url" parameter was empty.'));
  }

  // If we had the invoice_id, return the site status
  // NB: as a security measure, the invoice_id and url must match.
  if ($invoice_id) {
    $order_id = db_query('SELECT id FROM hosting_restapi_order WHERE invoice_id = :invoice_id AND site = :url', array(':invoice_id' => $invoice_id, ':url' => $url))->fetchField();
    $site = hosting_get_site_by_url($url);

    if (!$order_id) {
      throw new Exception(t('Invalid invoice ID or URL. Please contact support for more information.'));
    }

    return _hosting_restapi_get_site_install_status($site);
  }

  // Otherwise just return if the site exists or not.
  $site = hosting_get_site_by_url($url);

  if (!$site) {
    throw new Exception(t('Site not found. Please contact support for more information.'));
  }

  $result = [
    'status' => 'success',
    'data' => [
      'site_url' => $site->title,
      'site_status' => $site->site_status,
    ],
  ];

  return $result;
}

/**
 * Implements the 'site POST' API (create).
 *
 * NB: permission to create a site is done by checking the validity
 * of the invoice ($_POST['invoice']) in CiviCRM.
 * See @hosting_restapi_is_valid_invoice().
 *
 * The main domain ([url].example.org) is now enforced by hosting_saas, so
 * we only handle the [url] is rather the subdomain.
 */
function hosting_restapi_site_post() {
  $access = hosting_restapi_check_access($_POST['key'], $_POST['secret']);

  if (!$access) {
    throw new Exception(t('Access denied.'));
  }

  // Enforce a valid crmhost, otherwise we could validate invoices against a third-party endpoint
  if (empty($_POST['crmhost']) || !(in_array($_POST['crmhost'], ['https://civicrm.org', 'https://civicrm.com']))) {
    throw new Exception('Access denied.');
  }

  // Check if URL format is OK (i.e. no spaces, etc)
  $url = check_plain($_POST['url']);
  $url = preg_replace('/[^\.a-z0-9]/', '', $url);

  if ($url != $_POST['url']) {
    throw new Exception('The "url" parameter had invalid characters.');
  }

  $site = _hosting_restapi_is_allowed_creation($url);

  $invoice_id = $_POST['invoice'];

  // Log order in hosting_restapi_order
  $exists = db_query('SELECT count(*) as cpt FROM hosting_restapi_order WHERE invoice_id = :invoice_id', array(':invoice_id' => $invoice_id))->fetchField();

  if (!$exists) {
    $record = new StdClass();
    $record->invoice_id = $invoice_id;
    $record->site = $url;
    $record->token = sha1($url . uniqid(mt_rand()));
    $record->ip = $_SERVER['REMOTE_ADDR'];
    $record->crmhost = $_POST['crmhost'];
    $record->current_status = HOSTING_RESTAPI_ORDER_STATUS_NEW;
    $record->created = time();
    $record->updated = time();
    drupal_write_record('hosting_restapi_order', $record);
  }

  // Check with the remote crmhost to see if it is valid
  hosting_restapi_is_valid_invoice($invoice_id);

  // Create a site
  $new_platform_nid = variable_get('hosting_saas_target_platform', NULL);
  $new_db_server = variable_get('hosting_saas_db_server', 2); // 2 = DB server on single-server default setups

  if (hosting_restapi_check_capacity() === FALSE) {
    throw new Exception('Service has reached max capacity.');
  }

  $site = new Node();
  $site->type = 'site';
  $site->title = $url;
  $site->status = 1;
  $site->uid = 1;
  $site->site_status = HOSTING_SITE_QUEUED;
  $site->site_language = $_POST['language'];
  $site->platform = $new_platform_nid;
  $site->db_server = $new_db_server;
  $site->db_name = hosting_site_db_utils::suggest_db_name($url);
  $site->db_passwd = hosting_site_db_utils::generate_password();
  $site->environment = 1; /* prod */
  $site->cron_interval = 86400; /* 1 day */
  $site->civicrm_ansible_cron_interval = 900; /* 15 mins */

  $site = node_submit($site);
  node_save($site);

  hosting_restapi_log($url, HOSTING_RESTAPI_ORDER_STATUS_NEW, t('The order is valid and has been created.'));
  return [
    'status' => 'success',
    'data' => ['nid' => $site->nid],
  ];
}

/**
 * A few quick pre-flight checks to make sure that the system
 * is correctly configured (Drupal variales) and that the URL
 * is valid.
 */
function _hosting_restapi_is_allowed_creation($url) {
  if (!$url) {
    throw new Exception('The "url" parameter was empty.');
  }

  // This function checks for uniqueness of domains and aliases.
  // TODO: we should also implement hook_allow_domain() to enforce domain?
  if (! hosting_domain_allowed($url)) {
    // @todo [ML] does not work (above returns false), disable for now
    // throw new Exception('The url is not allowed by local configurations: ' . $url);
  }

  $site = hosting_get_site_by_url($url);

  if ($site) {
    throw new Exception('The site already exists.');
  }

  $variables = array(
    'hosting_saas_target_platform',
    'hosting_saas_db_server',
    'hosting_saas_template_site_nid',
  );

  foreach ($variables as $to_check) {
    if (variable_get($to_check, NULL) === NULL) {
      throw new Exception("Variable hasn't been chosen yet: " . $to_check);
    }
  }

  return $site;
}

/**
 * Checks if an invoice ID is valid (not already used, and valid in the transactional site).
 * i.e. checks if the invoice_id is valid in CiviCRM.
 *
 * @param String $invoice_id
 * @returns Boolean
 */
function hosting_restapi_is_valid_invoice($invoice_id) {
  if (! $invoice_id) {
    throw new Exception('Missing invoice.');
  }

  // Check if the invoice_id was already used.
  // Assumes that 1 invoice = 1 site
/*
  $exists = db_query('SELECT count(*) as cpt FROM hosting_restapi_order WHERE invoice_id = :invoice_id', array(':invoice_id' => $invoice_id))->fetchField();

  if ($exists) {
    watchdog('hosting_restapi', 'Invoice ID already used.');
    throw new Exception('Invalid invoice ID.');
  }
*/

  // NB: for now, we assume that hook implementations will throw an exception
  // if they couldn't validate the invoice_id. It assumes we have only one
  // way to validate invoices (create orders). If we want to change this at
  // some point, we should add a "invoice_type" argument.
  module_invoke_all("hosting_restapi_validate_invoice_id", $invoice_id);

  return TRUE;
}

/**
 *
 */
function _hosting_restapi_get_site_install_status($site) {
  $map = [
    'install' => HOSTING_RESTAPI_ORDER_STATUS_INSTALLED,
    'enable' => HOSTING_RESTAPI_ORDER_STATUS_ENABLED,
    'verify' => HOSTING_RESTAPI_ORDER_STATUS_VERIFIED,
  ];

  $task = db_query("SELECT *
    FROM hosting_task
    WHERE rid = :rid
      AND task_type IN ('install', 'enable', 'verify')
      AND task_status != 0
    ORDER BY executed DESC", [':rid' => $site->nid])->fetchObject();

  if (in_array($task->task_status, [HOSTING_TASK_ERROR, HOSTING_TASK_WARNING])) {
    return [
      'status' => 'error',
      'data' => [
        'site_url' => $site->title,
        'message' => "{$task->task_type} operation failed",
      ],
    ];
  }

  return [
    'status' => 'success',
    'data' => [
      'site_url' => $site->title,
      'site_status' => $map[$task->task_type] ?? HOSTING_RESTAPI_ORDER_STATUS_NEW,
    ],
  ];
}
