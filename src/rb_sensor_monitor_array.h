/*
  Copyright (C) 2016 Eneo Tecnologia S.L.
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

#pragma once

#include "rb_array.h"
#include "rb_sensor_monitor.h"

#include <json-c/json.h>

/// SNMP connection parameters
struct snmp_params_s {
  /// Peername to connect
  const char *peername;
  /// Connection values
  struct monitor_snmp_new_session_config session;
};

/// Monitors array
typedef struct rb_array rb_monitors_array_t;

/** Extract monitors array from a JSON array.
  @param monitors_array_json JSON monitors template
  @param sensor Sensor this monitor's belong
  @return New monitors array
  @note Need to free returned monitors with rb_monitors_array_done
  */
rb_monitors_array_t *parse_rb_monitors(struct json_object *monitors_array_json);

/// @todo Delete this FW declaration, we only need to use operation previous
/// values
struct rb_sensor;

/** Extract an indexed monitor of a monitor array
  @param array Monitors array
  @param i Index to extract
  @return Desired monitor
  */
rb_monitor_t *rb_monitors_array_elm_at(rb_monitors_array_t *array, size_t i);

/** Process all monitors in sensor, returning result in ret
  @param worker_info All workers info
  @param sensor_data Data of current sensor
  @param monitors Array of monitors to ask
  @param snmp_params SNMP connection parameters
  @param ret Message returning function
  @warning This function assumes ALL fields of sensor_data will be populated */
bool process_monitors_array(struct _worker_info * worker_info,
    struct rb_sensor_s *sensor, rb_monitors_array_t *monitors_array,
    struct snmp_params_s *snmp_params, rb_message_list *ret);

/** Free array allocated with parse_rb_monitors
  @param array Array
  */
void rb_monitors_array_done(rb_monitors_array_t *array);