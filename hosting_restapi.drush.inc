<?php

/**
 * @file
 *   Drush hooks for hosting_restapi.
 *
 * http://www.computerminds.co.uk/articles/storing-data-aegir
 */

/**
 * Implementation of hook_hosting_site_context_options().
 */
function hosting_restapi_hosting_site_context_options(&$task) {
  $token = db_query('SELECT token FROM hosting_restapi_order WHERE site = :url ORDER BY created DESC LIMIT 1', array(':url' => $task->ref->hosting_name))->fetchField();
  $task->context_options['hosting_restapi_token'] = $token;
  $task->context_options['hosting_restapi_hostmaster'] = variable_get('hosting_restapi_hostmaster', NULL);
}

/**
 * Implementation of hook_drush_context_import().
 * FIXME: this does not seem necessary? (it's never called?)
 */
function hosting_restapi_drush_context_import($context, &$node) {
  if ($context->type == 'site') {
    if (isset($context->hosting_restapi_token)) {
      $node->hosting_restapi_token = $context->hosting_restapi_token;
      $node->hosting_restapi_hostmaster = variable_get('hosting_restapi_hostmaster', NULL);
    }
  }
}
