<?php

/**
 *
 * This class allows to consume the API, either from within a module that knows civicrm already:
 *
 * @code
 *   require_once('api/class.api.php');
 *   $api = new civicrm_api3();
 * @endcode
 *
 * or from any code on the same server as civicrm
 *
 * @code
 *   require_once('/your/civi/folder/api/class.api.php');
 *   // the path to civicrm.settings.php
 *   $api = new civicrm_api3 (array('conf_path'=> '/your/path/to/your/civicrm/or/joomla/site));
 * @endcode
 *
 * or to query a remote server via the rest api
 *
 * @code
 *   $api = new civicrm_api3 (array ('server' => 'http://example.org',
 *                                   'api_key'=>'theusersecretkey',
 *                                   'site_key'=>'thesitesecretkey'));
 * @endcode
 *
 * No matter how initialised and if civicrm is local or remote, you use the class the same way.
 *
 * @code
 *   $api->{entity}->{action}($params);
 * @endcode
 *
 * So, to get the individual contacts:
 *
 * @code
 *   if ($api->Contact->Get(array('contact_type'=>'Individual','return'=>'sort_name,current_employer')) {
 *     // each key of the result array is an attribute of the api
 *     echo "\n contacts found " . $api->count;
 *     foreach ($api->values as $c) {
 *       echo "\n".$c->sort_name. " working for ". $c->current_employer;
 *     }
 *     // in theory, doesn't append
 *   } else {
 *     echo $api->errorMsg();
 *   }
 * @endcode
 *
 * Or, to create an event:
 *
 * @code
 *   if ($api->Event->Create(array('title'=>'Test','event_type_id' => 1,'is_public' => 1,'start_date' => 19430429))) {
 *     echo "created event id:". $api->id;
 *   } else {
 *     echo $api->errorMsg();
 *   }
 * @endcode
 *
 * To make it easier, the Actions can either take for input an
 * associative array $params, or simply an id. The following two lines
 * are equivalent.
 *
 * @code
 *   $api->Activity->Get (42);
 *   $api->Activity->Get (array('id'=>42));
 * @endcode
 *
 *
 * You can also get the result like civicrm_api does, but as an object
 * instead of an array (eg $entity->attribute instead of
 * $entity['attribute']).
 *
 * @code
 *   $result = $api->result;
 *   // is the json encoded result
 *   echo $api;
 * @endcode
 */
class civicrm_api3 {

  /**
   * @param array API configuration.
   */
  function __construct(Array $config) {
    $this->input      = [];
    $this->lastResult = [];
    if (empty($config['server'])) {
      throw new Exception('Missing server parameter');
    }
    $this->uri = $config['server'];
    if (!empty($config['path'])) {
      $this->uri .= "/" . $config['path'];
    }
    else {
      $this->uri .= '/civicrm/ajax/api4';
    }
    if (isset($config['site_key'])) {
      $this->site_key = $config['site_key'];
    }
    else {
      throw new Exception("param[key] missing");
    }
    if (isset($config['api_key'])) {
      $this->api_key = $config['api_key'];
    }
    else {
      throw new Exception("param[api_key] missing");
    }
  }

  /**
   *
   */
  public function __toString() {
    return json_encode($this->lastResult);
  }

  /**
   *
   */
  public function __call($action, $params) {
    // @TODO Check if it's a valid action.
    if (isset($params[0])) {
      return $this->call($this->currentEntity, $action, $params[0]);
    }
    else {
      return $this->call($this->currentEntity, $action, $this->input);
    }
  }

  /**
   *  As of PHP 5.3.0
   */
  public static function __callStatic($name, $arguments) {
    // Should we implement it ?
    echo "Calling static method '$name' " . implode(', ', $arguments) . "\n";
  }

  /**
   *
   */
  function remoteCall($entity, $action, $params = []) {
    // Ex: https://crm.example.org/civicrm/ajax/api4/Contact/get
    $url = $this->uri . '/' . $entity . '/' . $action;
    $request = stream_context_create([
      'http' => [
        'method' => 'POST',
        'header' => [
          'Content-Type: application/x-www-form-urlencoded',
          'X-Civi-Auth: Bearer ' . $this->api_key,
          'X-Civi-Key: ' . $this->site_key,
        ],
        'content' => http_build_query(['params' => json_encode($params)]),
      ]
    ]);
    $response = file_get_contents($url, FALSE, $request);
    $error = error_get_last();
    if (empty($response)) {
      // For some reason this does not seem to work when run from the web, maybe the Drupal error manager is catching it
      // tl;dr: usually if this is empty, it's because of an Api4 authentication error.
      error_log('hosting_restapi_civicrm: empty response (' . (empty($error) ? 'no error' : implode('; ', $error)) . ')');
      return NULL;
    }
    $result = json_decode($response, TRUE);
    return $result;
  }

  /**
   * @param $entity
   * @param string $action
   * @param array $params
   *
   * @return bool
   */
  function call($entity, $action = 'Get', $params = []) {
    $this->lastResult = $this->remoteCall($entity, $action, $params);

    // @todo [ML] Not very sure what this should return
    if ($this->lastResult == NULL) {
      return FALSE;
    }

    return TRUE;
  }

  /**
   * Return the last error message.
   */
  function errorMsg() {
    return $this->lastResult->error_message;
  }

  /**
   *
   */
  function init() {
    CRM_Core_DAO::init($this->cfg->dsn);
  }

  /**
   *
   */
  public function attr($name, $value = NULL) {
    if ($value === NULL) {
      if (property_exists($this->lastResult, $name)) {
        return $this->lastResult->$name;
      }
    }
    else {
      $this->input[$name] = $value;
    }
    return $this;
  }

  /**
   *
   */
  public function is_error() {
    // This used to check for is_error, but it does not seem to be available in api4?
    return empty($this->lastResult);
  }

  /**
   *
   */
  public function is_set($name) {
    return (isset($this->lastResult->$name));
  }

  /**
   *
   */
  public function __get($name) {
    // @TODO Test if valid entity.
    if (strtolower($name) !== $name) {
      // Cheap and dumb test to differentiate call to
      // $api->Entity->Action & value retrieval.
      $this->currentEntity = $name;
      return $this;
    }
    if ($name === 'result') {
      return $this->lastResult['values'];
    }
    if ($name === 'values') {
      return $this->lastResult['values'];
    }
    if (isset($this->lastResult[$name])) {
      return $this->lastResult[$name];
    }
    $this->currentEntity = $name;
    return $this;
  }

  /**
   * Or use $api->value.
   */
  public function values() {
    if (is_array($this->lastResult)) {
      return $this->lastResult['values'];
    }
    else {
      return $this->lastResult->values;
    }
  }

  /**
   * Or use $api->result.
   */
  public function result() {
    return $this->lastResult;
  }

}
