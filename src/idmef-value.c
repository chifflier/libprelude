/*****
*
* Copyright (C) 2002,2003 Krzysztof Zaraska <kzaraska@student.uci.agh.edu.pl>
* Copyright (C) 2003 Nicolas Delon <delon.nicolas@wanadoo.fr>
* Copyright (C) 2003 Yoann Vandoorselaere <yoann@prelude-ids.org>
* All Rights Reserved
*
* This file is part of the Prelude program.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****/

#include <inttypes.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "list.h"
#include "prelude-log.h"
#include "idmef-string.h"
#include "idmef.h"
#include "idmef-util.h"
#include "idmef-value-type.h"

#define CHUNK_SIZE 16


#define idmef_value_new(vname, vtype)                   \
idmef_value_t *idmef_value_new_ ## vname (vtype val) { 	\
	idmef_value_t *ptr;				\
                                                        \
	ptr = idmef_value_create(type_ ## vname);	\
	if ( ! ptr )			 	        \
		return NULL;			        \
						        \
	ptr->type.data. vname ## _val = val;            \
						        \
	return ptr;				        \
}


#define idmef_value_get_(vname, vtype)			\
vtype idmef_value_get_ ## vname (idmef_value_t *val)	\
{							\
	if ( val->type.id != type_ ## vname )	       	\
		return (vtype) 0;			\
							\
	return val->type.data. vname ## _val;		\
}



typedef struct compare {
	int result;	
	idmef_value_t *val2;
	idmef_relation_t relation;
} compare_t;



struct idmef_value {
    	int list_elems;
    	int list_max;
	int refcount;
	int own_data;
	idmef_value_t **list;
	idmef_value_type_t type;
	idmef_type_t object_type;
};



static int string_isdigit(const char *s)
{
	while ( *s ) {
		if ( ! isdigit(*s) )
			return -1;
		s++;
	}

	return 0;
}



static idmef_value_t *idmef_value_create(idmef_value_type_id_t type_id)
{
	idmef_value_t *ptr;
        
	ptr = calloc(1, sizeof(*ptr));
	if ( ! ptr ) {
		log(LOG_ERR, "memory exhausted.\n");
		return NULL;
	}
        
	ptr->refcount = 1;
	ptr->own_data = 1;
        ptr->type.id = type_id;
                
	return ptr;
}



idmef_value_new(int16, int16_t)
idmef_value_new(uint16, uint16_t)
idmef_value_new(int32, int32_t)
idmef_value_new(uint32, uint32_t)
idmef_value_new(int64, int64_t)
idmef_value_new(uint64, uint64_t)
idmef_value_new(float, float)
idmef_value_new(double, double)

idmef_value_get_(int16, int16_t)
idmef_value_get_(uint16, uint16_t)
idmef_value_get_(int32, int32_t)
idmef_value_get_(uint32, uint32_t)
idmef_value_get_(int64, int64_t)
idmef_value_get_(uint64, uint64_t)
idmef_value_get_(enum, int)
idmef_value_get_(float, float)
idmef_value_get_(double, double)
idmef_value_get_(string, idmef_string_t *)
idmef_value_get_(data, idmef_data_t *)
idmef_value_get_(time, idmef_time_t *)



idmef_value_t *idmef_value_new_string(idmef_string_t *string)
{
	idmef_value_t *ret;
	
	ret = idmef_value_create(type_string);
	if ( ! ret )
		return NULL;
	
	ret->type.data.string_val = string;
	
	return ret;
}



idmef_value_t *idmef_value_new_time(idmef_time_t *time)
{
	idmef_value_t *ret;

	ret = idmef_value_create(type_time);
	if ( ! ret )
		return NULL;

	ret->type.data.time_val = time;

	return ret;
}




idmef_value_t *idmef_value_new_data(idmef_data_t *data)
{
	idmef_value_t *ret;

	ret = idmef_value_create(type_data);
	if ( ! ret )
		return NULL;

	ret->type.data.data_val = data;

	return ret;
}




idmef_value_t *idmef_value_new_object(void *object, idmef_type_t object_type)
{
	idmef_value_t *ptr;				
						
	ptr = idmef_value_create(type_object);
	if ( ! ptr )
		return NULL;

	ptr->object_type = object_type;
	ptr->type.data.object_val = object;
	
	return ptr;	
}



idmef_value_t *idmef_value_new_enum_numeric(idmef_type_t type, int val)
{
	idmef_value_t *ptr;				
						
	ptr = idmef_value_create(type_enum);
	if ( ! ptr )
		return NULL;
        
	ptr->object_type = type;
	ptr->type.data.enum_val = val;

	return ptr;
}



idmef_value_t *idmef_value_new_enum_string(idmef_type_t type, const char *buf)
{
    	int val;

    	val = idmef_type_enum_to_numeric(type, buf);
    	if ( val < 0 )
	    	return NULL;
	
	return idmef_value_new_enum_numeric(type, val);

}



idmef_value_t *idmef_value_new_list(void)
{
	idmef_value_t *ptr;				

	ptr = idmef_value_create(type_object);
	if ( ! ptr )
		return NULL;

	ptr->list = malloc(CHUNK_SIZE * sizeof(idmef_value_t *));
	if ( ! ptr->list ) {
		log(LOG_ERR, "memory exhausted.\n");
	    	return NULL;
	}

	ptr->list_elems = 0;
	ptr->list_max = CHUNK_SIZE - 1;

	return ptr;	
}



int idmef_value_list_add(idmef_value_t *list, idmef_value_t *new)
{	
	if ( list->list_elems == list->list_max ) {
		list->list = realloc(list->list, (list->list_max + 1 + CHUNK_SIZE) *sizeof(idmef_value_t *));
	        if ( ! list->list ) {
                        log(LOG_ERR, "memory exhausted.\n");
                        return -1;
		}

                list->list_max += CHUNK_SIZE;
	}
	
	list->list[list->list_elems++] = new;
	
	return 0;
}



int idmef_value_is_list(idmef_value_t *list)
{
	return list ? (list->list ? 1 : 0) : -1;
}



int idmef_value_list_empty(idmef_value_t *list)
{		
	if ( ! list->list ) 
	    	return -1;
	
    	return (list->list_elems) ? 0 : 1;
}



idmef_value_t *idmef_value_new_enum(idmef_type_t type, const char *buf)
{
        if ( string_isdigit(buf) == 0 )
                return idmef_value_new_enum_numeric(type, atoi(buf));
        else
		return idmef_value_new_enum_string(type, buf);
}



idmef_value_t *idmef_value_new_generic(idmef_value_type_id_t type, const char *buf)
{
        int ret;
    	idmef_value_t *val;
        
    	val = idmef_value_create(type);
    	if ( ! val )
	    	return NULL;
        
        ret = idmef_value_type_read(&val->type, buf);
        if ( ret < 0 ) {
                free(val);
                return NULL;
        }
        
        val->own_data = 1;
        
	return val;
}



static int idmef_value_set_own_data(idmef_value_t *value, int own_data)
{
	int cnt;
        
	if ( value->list ) {
		for ( cnt = 0 ; cnt < value->list_elems; cnt++ )
			idmef_value_set_own_data(value->list[cnt], own_data);		
	} else 
		value->own_data = own_data;
	
	return 0;
}




int idmef_value_have_own_data(idmef_value_t *value)
{
	return idmef_value_set_own_data(value, 1);
}



int idmef_value_dont_have_own_data(idmef_value_t *value)
{
	return idmef_value_set_own_data(value, 0);
}



idmef_value_type_id_t idmef_value_get_type(idmef_value_t *value)
{
	return value->type.id;
}



idmef_type_t idmef_value_get_idmef_type(idmef_value_t *value)
{
	return (value->type.id == type_object) ? value->object_type : -1;
}



void *idmef_value_get_object(idmef_value_t *value)
{
	return (value->type.id == type_object) ? value->type.data.object_val : NULL;
		
}



int idmef_value_iterate(idmef_value_t *value, void *extra, int (*callback)(idmef_value_t *ptr, void *extra))
{
	int i, ret;
        
        if ( ! value->list )
                return callback(value, extra);
        
        for ( i = 0; i < value->list_elems; i++ ) {
                ret = callback(value->list[i], extra);
                if ( ret < 0 )
                        return -1;
        }
		
	return 0;
}




idmef_value_t *idmef_value_get_nth(idmef_value_t *val, int n)
{	
	if ( ! val->list )
	    	return (n == 0) ? val : NULL;
        
	return (n >= 0 && n < val->list_elems) ? val->list[n] : NULL;
}




int idmef_value_get_count(idmef_value_t *val)
{
	return val->list ? val->list_elems : 1;
}




static idmef_value_t *idmef_value_list_clone(idmef_value_t *val)
{
	idmef_value_t *new;
	int cnt;

	new = idmef_value_create(val->type.id);
	if ( ! new )
		return NULL;

	new->list_elems = val->list_elems;
	new->list_max = val->list_max;
	new->list = malloc((new->list_elems + 1) * sizeof (*new->list));

	for ( cnt = 0; cnt < new->list_elems; cnt++ ) {
		new->list[cnt] = idmef_value_clone(val->list[cnt]);
		if ( ! new->list[cnt] ) {

			while ( --cnt >= 0 )
				idmef_value_destroy(new->list[cnt]);
                }

                free(new->list);
                free(new);
                
                return NULL;
	}

	return new;
}



idmef_value_t *idmef_value_clone(idmef_value_t *val)
{
        int ret;
	idmef_value_t *new;
        
	if ( val->list )
		return idmef_value_list_clone(val);

	new = idmef_value_create(val->type.id);
	if ( ! new )
		return NULL;
	
        ret = idmef_value_type_clone(&new->type, &val->type);
        if ( ret < 0 ) {
                free(new);
                return NULL;
        }
        
	return new;
}



idmef_value_t *idmef_value_ref(idmef_value_t *val)
{	
	val->refcount++;
	
	return val;
}




int idmef_value_to_string(idmef_value_t *val, char *buf, int len)
{
	int ret;

        ret = idmef_value_type_write(buf, len, &val->type);
        
	return (ret < 0 || ret > len) ? -1 : ret;
}



int idmef_value_get(void *res, idmef_value_t *val)
{
	int ret;
        
        ret = idmef_value_type_copy(res, &val->type);
	if ( ret < 0 )
		return -1;
        
	return 0;
}



static int idmef_value_match_internal(idmef_value_t *val1, void *extra)
{
	idmef_value_t *val2;
	idmef_relation_t relation;
        compare_t *compare = extra;
        
        compare->result = -1;
	val2 = compare->val2;
	relation = compare->relation;

        if ( val1->type.id != val2->type.id ) 
		return -1;
        
        return idmef_value_type_compare(&val1->type, &val2->type, relation);
}



int idmef_value_match(idmef_value_t *val1, idmef_value_t *val2, idmef_relation_t relation)
{
	compare_t compare;
        
	compare.val2 = val2;
	compare.relation = relation;
        
	return idmef_value_iterate(val1, &compare, idmef_value_match_internal);
}



void idmef_value_destroy(idmef_value_t *val)
{
	int i;
	
	if ( --val->refcount )
	    	return;
	
	if ( val->list ) {
		for ( i = 0; i < val->list_elems; i++ )
			idmef_value_destroy(val->list[i]);
		
		free(val->list);
	}

	/*
         * Actual destructor starts here
         */
	if ( val->own_data )
		idmef_value_type_destroy(&val->type);

	free(val);
}
