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

* Aegir 6.x-2.x

License
=======

(C) 2014 Mathieu Lutfy <mathieu@bidon.ca>
(C) 2014 Coop SymbioTIC <info@symbiotic.coop>

Distributed under the terms of the GNU General public license (GPL).
See LICENSE.txt for details.
