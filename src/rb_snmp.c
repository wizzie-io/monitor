/*
  Copyright (C) 2016 Eneo Tecnologia S.L.
  Copyright (C) 2017 Eugenio Perez
  Author: Eugenio Perez <eupm90@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "rb_snmp.h"
#include <assert.h>
#include <librd/rd.h>
#include <librd/rdlog.h>

bool new_snmp_session(struct monitor_snmp_session *ss,
		      netsnmp_session *params) {

	ss->sessp = snmp_sess_open(params);
	if (unlikely(NULL == ss->sessp)) {
		char *strerror_buf = NULL;
		snmp_error(params, NULL, NULL, &strerror_buf);
		rdlog(LOG_ERR, "Failed to load SNMP session: %s", strerror_buf);
		free(strerror_buf);
	}

	return ss->sessp != NULL;
}

bool snmp_solve_response(char *value_buf,
			 size_t value_buf_len,
			 double *number,
			 struct monitor_snmp_session *session,
			 const char *oid_string) {
	struct snmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_GET);
	struct snmp_pdu *response = NULL;

	oid entry_oid[MAX_OID_LEN];
	size_t entry_oid_len = MAX_OID_LEN;
	read_objid(oid_string, entry_oid, &entry_oid_len);
	snmp_add_null_var(pdu, entry_oid, entry_oid_len);
	const int status = snmp_sess_synch_response(
			session->sessp, pdu, &response);
	/* A lot of variables. Just if we pass SNMPV3 someday.
	struct variable_list *vars;
	for(vars=response->variables; vars; vars=vars->next_variable)
		print_variable(vars->name,vars->name_length,vars);
	*/
	int ret = 0;
	assert(value_buf);
	assert(number);

	if (status != STAT_SUCCESS) {
		rdlog(LOG_ERR,
		      "Snmp error: %s",
		      snmp_api_errstring(snmp_sess_session(session->sessp)
							 ->s_snmp_errno));
		// rdlog(LOG_ERR,"Error in packet.Reason:
		// %s",snmp_errstring(response->errstat));
	} else if (NULL == response) {
		rdlog(LOG_ERR, "No SNMP response given.");
	} else {
		rdlog(LOG_DEBUG,
		      "SNMP OID %s response type %d: %s",
		      oid_string,
		      response->variables->type,
		      value_buf);
		const size_t effective_len = RD_MIN(
				value_buf_len, response->variables->val_len);

		// See in /usr/include/net-snmp/types.h
		switch (response->variables->type) {
		case ASN_GAUGE:
		case ASN_INTEGER:
			snprintf(value_buf,
				 value_buf_len,
				 "%ld",
				 *response->variables->val.integer);
			*number = *response->variables->val.integer;
			ret = 1;
			break;
		case ASN_OCTET_STR:
			if (effective_len == 0) {
				ret = 0;
				break;
			}

			snprintf(value_buf,
				 value_buf_len,
				 "%.*s",
				 (int)response->variables->val_len,
				 response->variables->val.string);

			*number = strtod(value_buf, NULL);
			ret = 1;
			break;

		default:
			rdlog(LOG_WARNING,
			      "Unknow variable type %d in SNMP response",
			      response->variables->type);
		};
	}

	if (response) {
		snmp_free_pdu(response);
	}
	return ret;
}

int net_snmp_version(const char *string_version, const char *sensor_name) {
	if (string_version) {
		if (0 == strcmp(string_version, "1")) {
			return SNMP_VERSION_1;
		}

		if (0 == strcmp(string_version, "2c")) {
			return SNMP_VERSION_2c;
		}
	}

	rdlog(LOG_ERR,
	      "Bad snmp version (%s) in sensor %s",
	      string_version,
	      sensor_name);
	return SNMP_DEFAULT_VERSION;
}

void destroy_snmp_session(struct monitor_snmp_session *s) {
	snmp_sess_close(s->sessp);
}
