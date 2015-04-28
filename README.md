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
