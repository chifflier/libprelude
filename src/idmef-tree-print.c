
/* Auto-generated by the GenerateIDMEFTreePrintC package (Thu Dec 25 16:32:38 2003) */

#include "idmef.h"
#include "idmef-tree-wrap.h"
#include "idmef-tree-print.h"
#include <stdio.h>
#include <time.h>

static int indent = 0;

static void print_indent(void)
{
	int cnt;

	for ( cnt = 0; cnt < indent; cnt++ )
		printf(" ");
}



static void print_string(idmef_string_t *string)
{
	const char *s;

	s = idmef_string_get_string(string);
	printf("%s", s ? s : "<empty>");
}



static void print_int16(int16_t i)
{
	printf("%hd", i);
}



static void print_uint16(uint16_t i)
{
	printf("%hu", i);
}



static void print_int32(int32_t i)
{
	printf("%d", i);
}



static void print_uint32(uint32_t i)
{
	printf("%u", i);
}



static void print_int64(int64_t i)
{
	printf("%lld", i);
}



static void print_uint64(uint64_t i)
{
	printf("%llu", i);
}



static void print_float(float f)
{
	printf("%f", f);
}




static void print_time(idmef_time_t *t)
{
	char buf[32];
	time_t _time;
	struct tm _tm;

	_time = idmef_time_get_sec(t);

	if ( ! localtime_r(&_time, &_tm) )
		return;

	if ( strftime(buf, sizeof (buf), "%H:%M:%S %d/%m/%Y", &_tm) == 0 )
		return;

	printf("%s (%u.%u)", buf, idmef_time_get_sec(t), idmef_time_get_usec(t));
}



/* print data as a string */

static void print_data(idmef_data_t *data)
{
	printf("%s", idmef_data_get_data(data));
}



static void print_enum(const char *s, int i)
{
	printf("%s (%d)", s, i);
}


void idmef_additional_data_print(idmef_additional_data_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		int i = idmef_additional_data_get_type(ptr);

		print_indent();
		printf("type: ");
		print_enum(idmef_additional_data_type_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_additional_data_get_meaning(ptr);

		if ( field ) {
			print_indent();
			printf("meaning: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_data_t *field;

		field = idmef_additional_data_get_data(ptr);

		if ( field ) {
			print_indent();
			printf("data: ");
			print_data(field);
			printf("\n");
		}
	}

	indent -= 8;
}

void idmef_classification_print(idmef_classification_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		int i = idmef_classification_get_origin(ptr);

		print_indent();
		printf("origin: ");
		print_enum(idmef_classification_origin_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_classification_get_name(ptr);

		if ( field ) {
			print_indent();
			printf("name: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_classification_get_url(ptr);

		if ( field ) {
			print_indent();
			printf("url: ");
			print_string(field);
			printf("\n");
		}
	}

	indent -= 8;
}

void idmef_userid_print(idmef_userid_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_userid_get_ident(ptr));
	printf("\n");

	{
		int i = idmef_userid_get_type(ptr);

		print_indent();
		printf("type: ");
		print_enum(idmef_userid_type_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_userid_get_name(ptr);

		if ( field ) {
			print_indent();
			printf("name: ");
			print_string(field);
			printf("\n");
		}
	}

	print_indent();
	printf("number: ");
	print_uint32(idmef_userid_get_number(ptr));
	printf("\n");

	indent -= 8;
}

void idmef_user_print(idmef_user_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_user_get_ident(ptr));
	printf("\n");

	{
		int i = idmef_user_get_category(ptr);

		print_indent();
		printf("category: ");
		print_enum(idmef_user_category_to_string(i), i);
		printf("\n");
        }

	{
		idmef_userid_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_user_get_next_userid(ptr, elem)) ) {
			print_indent();

			printf("userid(%d):\n", cnt);
			idmef_userid_print(elem);

			cnt++;
		}
	}

	indent -= 8;
}

void idmef_address_print(idmef_address_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_address_get_ident(ptr));
	printf("\n");

	{
		int i = idmef_address_get_category(ptr);

		print_indent();
		printf("category: ");
		print_enum(idmef_address_category_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_address_get_vlan_name(ptr);

		if ( field ) {
			print_indent();
			printf("vlan_name: ");
			print_string(field);
			printf("\n");
		}
	}

	print_indent();
	printf("vlan_num: ");
	print_uint32(idmef_address_get_vlan_num(ptr));
	printf("\n");

	{
		idmef_string_t *field;

		field = idmef_address_get_address(ptr);

		if ( field ) {
			print_indent();
			printf("address: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_address_get_netmask(ptr);

		if ( field ) {
			print_indent();
			printf("netmask: ");
			print_string(field);
			printf("\n");
		}
	}

	indent -= 8;
}

void idmef_process_print(idmef_process_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_process_get_ident(ptr));
	printf("\n");

	{
		idmef_string_t *field;

		field = idmef_process_get_name(ptr);

		if ( field ) {
			print_indent();
			printf("name: ");
			print_string(field);
			printf("\n");
		}
	}

	print_indent();
	printf("pid: ");
	print_uint32(idmef_process_get_pid(ptr));
	printf("\n");

	{
		idmef_string_t *field;

		field = idmef_process_get_path(ptr);

		if ( field ) {
			print_indent();
			printf("path: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_process_get_next_arg(ptr, elem)) ) {
			print_indent();

			printf("arg(%d): ", cnt);
			print_string(elem);
			printf("\n");

			cnt++;
		}
	}

	{
		idmef_string_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_process_get_next_env(ptr, elem)) ) {
			print_indent();

			printf("env(%d): ", cnt);
			print_string(elem);
			printf("\n");

			cnt++;
		}
	}

	indent -= 8;
}

void idmef_webservice_print(idmef_webservice_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		idmef_string_t *field;

		field = idmef_webservice_get_url(ptr);

		if ( field ) {
			print_indent();
			printf("url: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_webservice_get_cgi(ptr);

		if ( field ) {
			print_indent();
			printf("cgi: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_webservice_get_http_method(ptr);

		if ( field ) {
			print_indent();
			printf("http_method: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_webservice_get_next_arg(ptr, elem)) ) {
			print_indent();

			printf("arg(%d): ", cnt);
			print_string(elem);
			printf("\n");

			cnt++;
		}
	}

	indent -= 8;
}

void idmef_snmpservice_print(idmef_snmpservice_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		idmef_string_t *field;

		field = idmef_snmpservice_get_oid(ptr);

		if ( field ) {
			print_indent();
			printf("oid: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_snmpservice_get_community(ptr);

		if ( field ) {
			print_indent();
			printf("community: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_snmpservice_get_command(ptr);

		if ( field ) {
			print_indent();
			printf("command: ");
			print_string(field);
			printf("\n");
		}
	}

	indent -= 8;
}

void idmef_service_print(idmef_service_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_service_get_ident(ptr));
	printf("\n");

	{
		idmef_string_t *field;

		field = idmef_service_get_name(ptr);

		if ( field ) {
			print_indent();
			printf("name: ");
			print_string(field);
			printf("\n");
		}
	}

	print_indent();
	printf("port: ");
	print_uint16(idmef_service_get_port(ptr));
	printf("\n");

	{
		idmef_string_t *field;

		field = idmef_service_get_portlist(ptr);

		if ( field ) {
			print_indent();
			printf("portlist: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_service_get_protocol(ptr);

		if ( field ) {
			print_indent();
			printf("protocol: ");
			print_string(field);
			printf("\n");
		}
	}

	switch ( idmef_service_get_type(ptr) ) {
	case web_service:
		print_indent();
		printf("web:\n");
		idmef_webservice_print(idmef_service_get_web(ptr));
		break;
 
	case snmp_service:
		print_indent();
		printf("snmp:\n");
		idmef_snmpservice_print(idmef_service_get_snmp(ptr));
		break;
 
	default:
		break;
	}

	indent -= 8;
}

void idmef_node_print(idmef_node_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_node_get_ident(ptr));
	printf("\n");

	{
		int i = idmef_node_get_category(ptr);

		print_indent();
		printf("category: ");
		print_enum(idmef_node_category_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_node_get_location(ptr);

		if ( field ) {
			print_indent();
			printf("location: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_node_get_name(ptr);

		if ( field ) {
			print_indent();
			printf("name: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_address_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_node_get_next_address(ptr, elem)) ) {
			print_indent();

			printf("address(%d):\n", cnt);
			idmef_address_print(elem);

			cnt++;
		}
	}

	indent -= 8;
}

void idmef_source_print(idmef_source_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_source_get_ident(ptr));
	printf("\n");

	{
		int i = idmef_source_get_spoofed(ptr);

		print_indent();
		printf("spoofed: ");
		print_enum(idmef_spoofed_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_source_get_interface(ptr);

		if ( field ) {
			print_indent();
			printf("interface: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_node_t *field;

		field = idmef_source_get_node(ptr);

		if ( field ) {
			print_indent();
			printf("node:\n");
			idmef_node_print(field);
		}
	}	

	{
		idmef_user_t *field;

		field = idmef_source_get_user(ptr);

		if ( field ) {
			print_indent();
			printf("user:\n");
			idmef_user_print(field);
		}
	}	

	{
		idmef_process_t *field;

		field = idmef_source_get_process(ptr);

		if ( field ) {
			print_indent();
			printf("process:\n");
			idmef_process_print(field);
		}
	}	

	{
		idmef_service_t *field;

		field = idmef_source_get_service(ptr);

		if ( field ) {
			print_indent();
			printf("service:\n");
			idmef_service_print(field);
		}
	}	

	indent -= 8;
}

void idmef_file_access_print(idmef_file_access_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		idmef_userid_t *field;

		field = idmef_file_access_get_userid(ptr);

		if ( field ) {
			print_indent();
			printf("userid:\n");
			idmef_userid_print(field);
		}
	}	

	{
		idmef_string_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_file_access_get_next_permission(ptr, elem)) ) {
			print_indent();

			printf("permission(%d): ", cnt);
			print_string(elem);
			printf("\n");

			cnt++;
		}
	}

	indent -= 8;
}

void idmef_inode_print(idmef_inode_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		idmef_time_t *field;

		field = idmef_inode_get_change_time(ptr);

		if ( field ) {
			print_indent();
			printf("change_time: ");
			print_time(field);
			printf("\n");
		}
	}

	print_indent();
	printf("number: ");
	print_uint32(idmef_inode_get_number(ptr));
	printf("\n");

	print_indent();
	printf("major_device: ");
	print_uint32(idmef_inode_get_major_device(ptr));
	printf("\n");

	print_indent();
	printf("minor_device: ");
	print_uint32(idmef_inode_get_minor_device(ptr));
	printf("\n");

	print_indent();
	printf("c_major_device: ");
	print_uint32(idmef_inode_get_c_major_device(ptr));
	printf("\n");

	print_indent();
	printf("c_minor_device: ");
	print_uint32(idmef_inode_get_c_minor_device(ptr));
	printf("\n");

	indent -= 8;
}

void idmef_file_print(idmef_file_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_file_get_ident(ptr));
	printf("\n");

	{
		int i = idmef_file_get_category(ptr);

		print_indent();
		printf("category: ");
		print_enum(idmef_file_category_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_file_get_fstype(ptr);

		if ( field ) {
			print_indent();
			printf("fstype: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_file_get_name(ptr);

		if ( field ) {
			print_indent();
			printf("name: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_file_get_path(ptr);

		if ( field ) {
			print_indent();
			printf("path: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_time_t *field;

		field = idmef_file_get_create_time(ptr);

		if ( field ) {
			print_indent();
			printf("create_time: ");
			print_time(field);
			printf("\n");
		}
	}

	{
		idmef_time_t *field;

		field = idmef_file_get_modify_time(ptr);

		if ( field ) {
			print_indent();
			printf("modify_time: ");
			print_time(field);
			printf("\n");
		}
	}

	{
		idmef_time_t *field;

		field = idmef_file_get_access_time(ptr);

		if ( field ) {
			print_indent();
			printf("access_time: ");
			print_time(field);
			printf("\n");
		}
	}

	print_indent();
	printf("data_size: ");
	print_uint32(idmef_file_get_data_size(ptr));
	printf("\n");

	print_indent();
	printf("disk_size: ");
	print_uint32(idmef_file_get_disk_size(ptr));
	printf("\n");

	{
		idmef_file_access_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_file_get_next_file_access(ptr, elem)) ) {
			print_indent();

			printf("file_access(%d):\n", cnt);
			idmef_file_access_print(elem);

			cnt++;
		}
	}

	{
		idmef_linkage_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_file_get_next_file_linkage(ptr, elem)) ) {
			print_indent();

			printf("file_linkage(%d):\n", cnt);
			idmef_linkage_print(elem);

			cnt++;
		}
	}

	{
		idmef_inode_t *field;

		field = idmef_file_get_inode(ptr);

		if ( field ) {
			print_indent();
			printf("inode:\n");
			idmef_inode_print(field);
		}
	}	

	indent -= 8;
}

void idmef_linkage_print(idmef_linkage_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		int i = idmef_linkage_get_category(ptr);

		print_indent();
		printf("category: ");
		print_enum(idmef_linkage_category_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_linkage_get_name(ptr);

		if ( field ) {
			print_indent();
			printf("name: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_linkage_get_path(ptr);

		if ( field ) {
			print_indent();
			printf("path: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_file_t *field;

		field = idmef_linkage_get_file(ptr);

		if ( field ) {
			print_indent();
			printf("file:\n");
			idmef_file_print(field);
		}
	}	

	indent -= 8;
}

void idmef_target_print(idmef_target_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_target_get_ident(ptr));
	printf("\n");

	{
		int i = idmef_target_get_decoy(ptr);

		print_indent();
		printf("decoy: ");
		print_enum(idmef_spoofed_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_target_get_interface(ptr);

		if ( field ) {
			print_indent();
			printf("interface: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_node_t *field;

		field = idmef_target_get_node(ptr);

		if ( field ) {
			print_indent();
			printf("node:\n");
			idmef_node_print(field);
		}
	}	

	{
		idmef_user_t *field;

		field = idmef_target_get_user(ptr);

		if ( field ) {
			print_indent();
			printf("user:\n");
			idmef_user_print(field);
		}
	}	

	{
		idmef_process_t *field;

		field = idmef_target_get_process(ptr);

		if ( field ) {
			print_indent();
			printf("process:\n");
			idmef_process_print(field);
		}
	}	

	{
		idmef_service_t *field;

		field = idmef_target_get_service(ptr);

		if ( field ) {
			print_indent();
			printf("service:\n");
			idmef_service_print(field);
		}
	}	

	{
		idmef_file_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_target_get_next_file(ptr, elem)) ) {
			print_indent();

			printf("file(%d):\n", cnt);
			idmef_file_print(elem);

			cnt++;
		}
	}

	indent -= 8;
}

void idmef_analyzer_print(idmef_analyzer_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("analyzerid: ");
	print_uint64(idmef_analyzer_get_analyzerid(ptr));
	printf("\n");

	{
		idmef_string_t *field;

		field = idmef_analyzer_get_manufacturer(ptr);

		if ( field ) {
			print_indent();
			printf("manufacturer: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_analyzer_get_model(ptr);

		if ( field ) {
			print_indent();
			printf("model: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_analyzer_get_version(ptr);

		if ( field ) {
			print_indent();
			printf("version: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_analyzer_get_class(ptr);

		if ( field ) {
			print_indent();
			printf("class: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_analyzer_get_ostype(ptr);

		if ( field ) {
			print_indent();
			printf("ostype: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_analyzer_get_osversion(ptr);

		if ( field ) {
			print_indent();
			printf("osversion: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_node_t *field;

		field = idmef_analyzer_get_node(ptr);

		if ( field ) {
			print_indent();
			printf("node:\n");
			idmef_node_print(field);
		}
	}	

	{
		idmef_process_t *field;

		field = idmef_analyzer_get_process(ptr);

		if ( field ) {
			print_indent();
			printf("process:\n");
			idmef_process_print(field);
		}
	}	

	indent -= 8;
}

void idmef_alertident_print(idmef_alertident_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("alertident: ");
	print_uint64(idmef_alertident_get_alertident(ptr));
	printf("\n");

	print_indent();
	printf("analyzerid: ");
	print_uint64(idmef_alertident_get_analyzerid(ptr));
	printf("\n");

	indent -= 8;
}

void idmef_impact_print(idmef_impact_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		int i = idmef_impact_get_severity(ptr);

		print_indent();
		printf("severity: ");
		print_enum(idmef_impact_severity_to_string(i), i);
		printf("\n");
        }

	{
		int i = idmef_impact_get_completion(ptr);

		print_indent();
		printf("completion: ");
		print_enum(idmef_impact_completion_to_string(i), i);
		printf("\n");
        }

	{
		int i = idmef_impact_get_type(ptr);

		print_indent();
		printf("type: ");
		print_enum(idmef_impact_type_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_impact_get_description(ptr);

		if ( field ) {
			print_indent();
			printf("description: ");
			print_string(field);
			printf("\n");
		}
	}

	indent -= 8;
}

void idmef_action_print(idmef_action_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		int i = idmef_action_get_category(ptr);

		print_indent();
		printf("category: ");
		print_enum(idmef_action_category_to_string(i), i);
		printf("\n");
        }

	{
		idmef_string_t *field;

		field = idmef_action_get_description(ptr);

		if ( field ) {
			print_indent();
			printf("description: ");
			print_string(field);
			printf("\n");
		}
	}

	indent -= 8;
}

void idmef_confidence_print(idmef_confidence_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		int i = idmef_confidence_get_rating(ptr);

		print_indent();
		printf("rating: ");
		print_enum(idmef_confidence_rating_to_string(i), i);
		printf("\n");
        }

	print_indent();
	printf("confidence: ");
	print_float(idmef_confidence_get_confidence(ptr));
	printf("\n");

	indent -= 8;
}

void idmef_assessment_print(idmef_assessment_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		idmef_impact_t *field;

		field = idmef_assessment_get_impact(ptr);

		if ( field ) {
			print_indent();
			printf("impact:\n");
			idmef_impact_print(field);
		}
	}	

	{
		idmef_action_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_assessment_get_next_action(ptr, elem)) ) {
			print_indent();

			printf("action(%d):\n", cnt);
			idmef_action_print(elem);

			cnt++;
		}
	}

	{
		idmef_confidence_t *field;

		field = idmef_assessment_get_confidence(ptr);

		if ( field ) {
			print_indent();
			printf("confidence:\n");
			idmef_confidence_print(field);
		}
	}	

	indent -= 8;
}

void idmef_tool_alert_print(idmef_tool_alert_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		idmef_string_t *field;

		field = idmef_tool_alert_get_name(ptr);

		if ( field ) {
			print_indent();
			printf("name: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_string_t *field;

		field = idmef_tool_alert_get_command(ptr);

		if ( field ) {
			print_indent();
			printf("command: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_alertident_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_tool_alert_get_next_alertident(ptr, elem)) ) {
			print_indent();

			printf("alertident(%d):\n", cnt);
			idmef_alertident_print(elem);

			cnt++;
		}
	}

	indent -= 8;
}

void idmef_correlation_alert_print(idmef_correlation_alert_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		idmef_string_t *field;

		field = idmef_correlation_alert_get_name(ptr);

		if ( field ) {
			print_indent();
			printf("name: ");
			print_string(field);
			printf("\n");
		}
	}

	{
		idmef_alertident_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_correlation_alert_get_next_alertident(ptr, elem)) ) {
			print_indent();

			printf("alertident(%d):\n", cnt);
			idmef_alertident_print(elem);

			cnt++;
		}
	}

	indent -= 8;
}

void idmef_overflow_alert_print(idmef_overflow_alert_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		idmef_string_t *field;

		field = idmef_overflow_alert_get_program(ptr);

		if ( field ) {
			print_indent();
			printf("program: ");
			print_string(field);
			printf("\n");
		}
	}

	print_indent();
	printf("size: ");
	print_uint32(idmef_overflow_alert_get_size(ptr));
	printf("\n");

	{
		idmef_data_t *field;

		field = idmef_overflow_alert_get_buffer(ptr);

		if ( field ) {
			print_indent();
			printf("buffer: ");
			print_data(field);
			printf("\n");
		}
	}

	indent -= 8;
}

void idmef_alert_print(idmef_alert_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_alert_get_ident(ptr));
	printf("\n");

	{
		idmef_assessment_t *field;

		field = idmef_alert_get_assessment(ptr);

		if ( field ) {
			print_indent();
			printf("assessment:\n");
			idmef_assessment_print(field);
		}
	}	

	{
		idmef_analyzer_t *field;

		field = idmef_alert_get_analyzer(ptr);

		if ( field ) {
			print_indent();
			printf("analyzer:\n");
			idmef_analyzer_print(field);
		}
	}	

	{
		idmef_time_t *field;

		field = idmef_alert_get_create_time(ptr);

		if ( field ) {
			print_indent();
			printf("create_time: ");
			print_time(field);
			printf("\n");
		}
	}

	{
		idmef_time_t *field;

		field = idmef_alert_get_detect_time(ptr);

		if ( field ) {
			print_indent();
			printf("detect_time: ");
			print_time(field);
			printf("\n");
		}
	}

	{
		idmef_time_t *field;

		field = idmef_alert_get_analyzer_time(ptr);

		if ( field ) {
			print_indent();
			printf("analyzer_time: ");
			print_time(field);
			printf("\n");
		}
	}

	{
		idmef_source_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_alert_get_next_source(ptr, elem)) ) {
			print_indent();

			printf("source(%d):\n", cnt);
			idmef_source_print(elem);

			cnt++;
		}
	}

	{
		idmef_target_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_alert_get_next_target(ptr, elem)) ) {
			print_indent();

			printf("target(%d):\n", cnt);
			idmef_target_print(elem);

			cnt++;
		}
	}

	{
		idmef_classification_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_alert_get_next_classification(ptr, elem)) ) {
			print_indent();

			printf("classification(%d):\n", cnt);
			idmef_classification_print(elem);

			cnt++;
		}
	}

	{
		idmef_additional_data_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_alert_get_next_additional_data(ptr, elem)) ) {
			print_indent();

			printf("additional_data(%d):\n", cnt);
			idmef_additional_data_print(elem);

			cnt++;
		}
	}

	switch ( idmef_alert_get_type(ptr) ) {
	case idmef_tool_alert:
		print_indent();
		printf("tool_alert:\n");
		idmef_tool_alert_print(idmef_alert_get_tool_alert(ptr));
		break;
 
	case idmef_correlation_alert:
		print_indent();
		printf("correlation_alert:\n");
		idmef_correlation_alert_print(idmef_alert_get_correlation_alert(ptr));
		break;
 
	case idmef_overflow_alert:
		print_indent();
		printf("overflow_alert:\n");
		idmef_overflow_alert_print(idmef_alert_get_overflow_alert(ptr));
		break;
 
	default:
		break;
	}

	indent -= 8;
}

void idmef_heartbeat_print(idmef_heartbeat_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	print_indent();
	printf("ident: ");
	print_uint64(idmef_heartbeat_get_ident(ptr));
	printf("\n");

	{
		idmef_analyzer_t *field;

		field = idmef_heartbeat_get_analyzer(ptr);

		if ( field ) {
			print_indent();
			printf("analyzer:\n");
			idmef_analyzer_print(field);
		}
	}	

	{
		idmef_time_t *field;

		field = idmef_heartbeat_get_create_time(ptr);

		if ( field ) {
			print_indent();
			printf("create_time: ");
			print_time(field);
			printf("\n");
		}
	}

	{
		idmef_time_t *field;

		field = idmef_heartbeat_get_analyzer_time(ptr);

		if ( field ) {
			print_indent();
			printf("analyzer_time: ");
			print_time(field);
			printf("\n");
		}
	}

	{
		idmef_additional_data_t *elem = NULL;
		int cnt = 0;

		while ( (elem = idmef_heartbeat_get_next_additional_data(ptr, elem)) ) {
			print_indent();

			printf("additional_data(%d):\n", cnt);
			idmef_additional_data_print(elem);

			cnt++;
		}
	}

	indent -= 8;
}

void idmef_message_print(idmef_message_t *ptr)
{
	if ( ! ptr )
		return;

	indent += 8;

	{
		idmef_string_t *field;

		field = idmef_message_get_version(ptr);

		if ( field ) {
			print_indent();
			printf("version: ");
			print_string(field);
			printf("\n");
		}
	}

	switch ( idmef_message_get_type(ptr) ) {
	case idmef_alert_message:
		print_indent();
		printf("alert:\n");
		idmef_alert_print(idmef_message_get_alert(ptr));
		break;
 
	case idmef_heartbeat_message:
		print_indent();
		printf("heartbeat:\n");
		idmef_heartbeat_print(idmef_message_get_heartbeat(ptr));
		break;
 
	default:
		break;
	}

	indent -= 8;
}
