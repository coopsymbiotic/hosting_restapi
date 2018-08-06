This is an experimental, badly named module, for a very specific internal
requirement to link a CiviCRM contribution page and an Aegir front-end.

* the user goes through a "contribution" payment page to sign-up to Aegir
* he gets an invoice_id that is then sent (using ajax) to the Aegir front-end
* the invoice_id is validated using the CiviCRM REST API
* a site is cloned to the requrested URL
* the user can poll the Aegir front-end to know the status of the clone operation.

To download the latest version of this module:
https://github.com/coopsymbiotic/hosting_restapi

Requirements
============

* Aegir 7.x-3.x
* hosting_saas https://drupal.org/project/hosting_saas

Recommended
===========

This module was initially developped for Coop SymbioTIC's CiviCRM hosting.
It was therefore designed to work with a CiviCRM contribution page to track
payments and to track client/organisation information.

* Front end is in the symbiocivicrm extension:
  https://github.com/coopsymbiotic/coop.symbiotic.symbiocivicrm

* Post-install site configuration is in provision_symbiotic:
  https://github.com/coopsymbiotic/provision_symbiotic 

Supported use cases and assumptions
===================================

Simplest use case
-----------------

You need a REST API to create sites in Aegir. It might be because you're using
the command line and need to script things, or you want to tie in with another
admin software.

In this case, you only need to authenticate your external tool using a single
API key.

Integration with a payment form (ex: CiviCRM, Webform or Commerce)
------------------------------------------------------------------

You would like your users to self-serve signup to your services using a web
based form. That form may include payment up front (credit card or other).

In this case, since we wanted the workflow to be interactive for clients
and avoid proxying requests through the web server, clients can submit their
REST request directly to the Aegir server.

To authenticate requets, clients must provide their invoice ID and URL.
The Aegir server then authenticates back with the front-end to validate
the invoice ID (is paid, has not been re-used). Each valid order gets
assigned a token. The token is then used to authenticate the site for
further requests (ex: get org/client information, run backups, get db, etc).

Getting started with CiviCRM
============================

* Install this module in your Aegir front-end (under `/var/aegir/hostmaster-7.x-3.xx/sites/example.org/modules/`).
* Also enable `hosting_restapi_civicrm`
* Create an API user on your CiviCRM instance (where signups are handled).
* Set the API credentials on your Aegir server:

```
drush @hm vset hosting_restapi_crmhost https://crm.example.org
drush @hm vset hosting_restapi_crmapikey YOUR_USER_API_KEY
drush @hm vset hosting_restapi_crmkey YOUR_CIVICRM_SITE_KEY
```

NB: currently it assumes that you are using Drupal7 and that CiviCRM is in `/sites/all/modules/civicrm/`. This can easily be fixed in `hosting_restapi_civicrm_api` by adding the `path` parameter to `civicrm_api3` (see in `civicrm.class.php`).

API documentation
=================

(needs more information on parameters, etc)

* GET /hosting/api/site : get information on a site. Arguments: url, token.

* POST /hosting/api/site : create a new site. Arguments: url, invoice_id.

* GET /hosting/api/sites/config : get site config (org name, contact info, etc). Arguments: url, token.

TODO (not yet implemented, brainstorm):

* run a backup
* download a copy of the backup
* clone/sync a site? (prod->dev)

License
=======

(C) 2014-2015 Mathieu Lutfy <mathieu@bidon.ca>
(C) 2014-2015 Coop SymbioTIC <info@symbiotic.coop>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

See LICENSE.txt for details.

A license exception is granted to run this program in combination with
PHP. See LICENSE.exception.txt for details.
