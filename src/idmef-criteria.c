/*****
*
* Copyright (C) 2004-2005 Yoann Vandoorselaere <yoann@prelude-ids.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdarg.h>
#include <assert.h>

#include "libmissing.h"
#include "prelude-log.h"
#include "prelude-error.h"
#include "prelude-inttypes.h"

#include "idmef.h"
#include "idmef-criteria.h"


struct idmef_criterion {
	idmef_path_t *path;
	idmef_criterion_value_t *value;
	idmef_value_relation_t relation;
};


struct idmef_criteria {
	prelude_list_t list;
        
	idmef_criterion_t *criterion;
        struct idmef_criteria *or;
        struct idmef_criteria *and;
};



static int idmef_criterion_check_path_value(idmef_path_t *path,
                                            idmef_criterion_value_t *value)
{
        int ret = 0;
        idmef_value_t *fixed;
	idmef_value_type_id_t type;

	type = idmef_path_get_value_type(path);

	switch ( idmef_criterion_value_get_type(value) ) {
                
	case IDMEF_CRITERION_VALUE_TYPE_FIXED: 
		fixed = idmef_criterion_value_get_fixed(value);
		ret = (idmef_value_get_type(fixed) == type);
		break;

	case IDMEF_CRITERION_VALUE_TYPE_NON_LINEAR_TIME:
		ret = (type == IDMEF_VALUE_TYPE_TIME);
		break;
                
	}

	return ret ? 0 : -1;
}



static int is_non_linear_time_contiguous(idmef_criterion_value_non_linear_time_t *time)
{
	int time_parts[6];
	int cnt;
	int start;
	
	time_parts[0] = idmef_criterion_value_non_linear_time_get_year(time);
	time_parts[1] = idmef_criterion_value_non_linear_time_get_month(time);
	time_parts[2] = idmef_criterion_value_non_linear_time_get_mday(time);
	time_parts[3] = idmef_criterion_value_non_linear_time_get_hour(time);
	time_parts[4] = idmef_criterion_value_non_linear_time_get_min(time);
	time_parts[5] = idmef_criterion_value_non_linear_time_get_sec(time);

	if ( time_parts[0] != -1 )
		start = 0;

	else if ( time_parts[3] != -1 )
		start = 3;

	else
		return 0;

	for ( cnt = 0; cnt < start; cnt++ )
		if ( time_parts[cnt] != -1 )
			return 0;

	for ( cnt = start + 1;
	      cnt < sizeof (time_parts) / sizeof (time_parts[0]) && time_parts[cnt] != -1;
	      cnt++ );

	for ( cnt += 1; cnt < sizeof (time_parts) / sizeof (time_parts[0]); cnt++ )
		if ( time_parts[cnt] != -1 )
			return 0;

	return 1;
}



static int count_non_linear_time_parts(idmef_criterion_value_non_linear_time_t *time)
{
	return ((idmef_criterion_value_non_linear_time_get_year(time) != -1) +
		(idmef_criterion_value_non_linear_time_get_month(time) != -1) +
		(idmef_criterion_value_non_linear_time_get_yday(time) != -1) +
		(idmef_criterion_value_non_linear_time_get_mday(time) != -1) +
		(idmef_criterion_value_non_linear_time_get_wday(time) != -1) +
		(idmef_criterion_value_non_linear_time_get_hour(time) != -1) +
		(idmef_criterion_value_non_linear_time_get_min(time) != -1) +
		(idmef_criterion_value_non_linear_time_get_sec(time) != -1));
}



static int idmef_criterion_check_relation_value_non_linear_value(idmef_criterion_value_non_linear_time_t *time,
								 idmef_value_relation_t relation)
{
	int time_parts_count;

	if ( relation & IDMEF_VALUE_RELATION_SUBSTR    ||
             relation & IDMEF_VALUE_RELATION_IS_NULL   ||
             relation & IDMEF_VALUE_RELATION_IS_NOT_NULL )
		return -1;

	if ( relation == IDMEF_VALUE_RELATION_EQUAL || relation == IDMEF_VALUE_RELATION_NOT_EQUAL )
		return 0;

	time_parts_count = count_non_linear_time_parts(time);
	if ( time_parts_count == 0 )
		return -1;

	if ( ( idmef_criterion_value_non_linear_time_get_yday(time) != -1 ||
	       idmef_criterion_value_non_linear_time_get_wday(time) != -1 ||
	       idmef_criterion_value_non_linear_time_get_month(time) != -1 ||
	       idmef_criterion_value_non_linear_time_get_mday(time) != -1 ) &&
	     time_parts_count == 1 )
		return 0;

	return is_non_linear_time_contiguous(time) ? 0 : -1;
}



static int idmef_criterion_check_relation(idmef_criterion_value_t *value,
					  idmef_value_relation_t relation)
{
	switch ( idmef_criterion_value_get_type(value) ) {

        case IDMEF_CRITERION_VALUE_TYPE_FIXED:
		return idmef_value_check_relation(idmef_criterion_value_get_fixed(value), relation);

	case IDMEF_CRITERION_VALUE_TYPE_NON_LINEAR_TIME:
		return idmef_criterion_check_relation_value_non_linear_value
			(idmef_criterion_value_get_non_linear_time(value), relation);
	}

	/* never reached */
	return -1;
}



static int idmef_criterion_check(idmef_path_t *path,
				 idmef_criterion_value_t *value,
                                 idmef_value_relation_t relation)
{
	if ( idmef_criterion_check_path_value(path, value) < 0 )
		return -1;
        
	return idmef_criterion_check_relation(value, relation);
}



int idmef_criterion_new(idmef_criterion_t **criterion, idmef_path_t *path,
                        idmef_criterion_value_t *value, idmef_value_relation_t relation)
{
        int ret;
        
        if ( value && (ret = idmef_criterion_check(path, value, relation)) < 0 )
		return ret;

	*criterion = calloc(1, sizeof(**criterion));
	if ( ! *criterion )
		return prelude_error_from_errno(errno);

	(*criterion)->path = path;
	(*criterion)->relation = relation;
	(*criterion)->value = value;

	return 0;
}



void idmef_criterion_destroy(idmef_criterion_t *criterion)
{
	idmef_path_destroy(criterion->path);

	if ( criterion->value ) /* can be NULL if relation is is_null or is_not_null */
		idmef_criterion_value_destroy(criterion->value);

	free(criterion);
}



int idmef_criterion_clone(idmef_criterion_t *criterion, idmef_criterion_t **dst)
{
        int ret;
	idmef_path_t *path;
	idmef_criterion_value_t *value = NULL;

	ret = idmef_path_clone(criterion->path, &path);
	if ( ret < 0 )
		return ret;

	if ( criterion->value ) {
		ret = idmef_criterion_value_clone(criterion->value, &value);
		if ( ret < 0 ) {
			idmef_path_destroy(path);
			return ret;
		}
	} 

	ret = idmef_criterion_new(dst, path, value, criterion->relation);
	if ( ret < 0 ) {
		idmef_path_destroy(path);
		idmef_criterion_value_destroy(value);
		return ret;
	}

	return 0;
}



int idmef_criterion_print(const idmef_criterion_t *criterion, prelude_io_t *fd)
{
        int ret;
        prelude_string_t *out;

        ret = prelude_string_new(&out);
        if ( ret < 0 )
                return ret;

        ret = idmef_criterion_to_string(criterion, out);
        if ( ret < 0 ) {
                prelude_string_destroy(out);
                return ret;
        }

        ret = prelude_io_write(fd, prelude_string_get_string(out), prelude_string_get_len(out));
        prelude_string_destroy(out);

        return ret;
}



int idmef_criterion_to_string(const idmef_criterion_t *criterion, prelude_string_t *out)
{
        const char *operator;

        operator = idmef_value_relation_to_string(criterion->relation);
        assert(operator);

        if ( ! criterion->value )
                return prelude_string_sprintf(out, "%s %s", operator, idmef_path_get_name(criterion->path));

        prelude_string_sprintf(out, "%s %s ", idmef_path_get_name(criterion->path), operator);

        return idmef_criterion_value_to_string(criterion->value, out);
}



idmef_path_t *idmef_criterion_get_path(idmef_criterion_t *criterion)
{
	return criterion->path;
}



idmef_criterion_value_t *idmef_criterion_get_value(idmef_criterion_t *criterion)
{
	return criterion ? criterion->value : NULL;
}



idmef_value_relation_t idmef_criterion_get_relation(idmef_criterion_t *criterion)
{
        return criterion->relation;
}



/* 
 * Check if message matches criterion. Returns 1 if it does, 0 if it doesn't,
 * -1 on error (e.g. a comparision with NULL)
 */
int idmef_criterion_match(idmef_criterion_t *criterion, idmef_message_t *message)
{
        int ret = 0;
	idmef_value_t *value;
	
	ret = idmef_path_get(criterion->path, message, &value);
        if ( ret < 0 )
                return ret;
        
        if ( ret == 0 ) 
		return (criterion->relation == IDMEF_VALUE_RELATION_IS_NULL) ? 0 : -1;
        
        if ( ! criterion->value )
                return (criterion->relation == IDMEF_VALUE_RELATION_IS_NOT_NULL) ? 0 : -1; 
        
	switch ( idmef_criterion_value_get_type(criterion->value) ) {
                
        case IDMEF_CRITERION_VALUE_TYPE_FIXED:
        	ret = idmef_value_match(value,
                                        idmef_criterion_value_get_fixed(criterion->value),
                                        criterion->relation);
		break;
		
	case IDMEF_CRITERION_VALUE_TYPE_NON_LINEAR_TIME:
		/* not supported for the moment */
                ret = -1;
		break;
	}

	idmef_value_destroy(value);

	return ret;
}



int idmef_criteria_new(idmef_criteria_t **criteria)
{
        *criteria = calloc(1, sizeof(**criteria));
	if ( ! *criteria )
		return prelude_error_from_errno(errno);

        (*criteria)->or = NULL;
        (*criteria)->and = NULL;

	return 0;
}



void idmef_criteria_destroy(idmef_criteria_t *criteria)
{
	if ( criteria->criterion )
		idmef_criterion_destroy(criteria->criterion);

	if ( criteria->or )
		idmef_criteria_destroy(criteria->or);

	if ( criteria->and )
		idmef_criteria_destroy(criteria->and);
        
	free(criteria);
}



int idmef_criteria_clone(idmef_criteria_t *src, idmef_criteria_t **dst)
{
        int ret;
        idmef_criteria_t *new;
        
        new = *dst = malloc(sizeof(*new));
        if ( ! new )
                return prelude_error_from_errno(errno);

        memcpy(new, src, sizeof(*new));

	if ( src->or ) {
		ret = idmef_criteria_clone(src->or, &new->or);
		if ( ret < 0 ) {
			idmef_criteria_destroy(new);
			return ret;
		}
	}

	if ( src->and ) {
		ret = idmef_criteria_clone(src->and, &new->and);
		if ( ret < 0 ) {
			idmef_criteria_destroy(new);
			return ret;
		}
	}
        
        ret = idmef_criterion_clone(src->criterion, &new->criterion);
        if ( ret < 0 ) {
                idmef_criteria_destroy(new);
                return ret;
        }
        
        return 0;
}



idmef_criteria_t *idmef_criteria_get_or(idmef_criteria_t *criteria)
{
        return criteria->or;
}



idmef_criteria_t *idmef_criteria_get_and(idmef_criteria_t *criteria)
{
        return criteria->and;
}



int idmef_criteria_print(idmef_criteria_t *criteria, prelude_io_t *fd)
{
        int ret;
        prelude_string_t *out;

        ret = prelude_string_new(&out);
        if ( ret < 0 )
                return ret;

        ret = idmef_criteria_to_string(criteria, out);
        if ( ret < 0 )
                return ret;

        ret = prelude_io_write(fd, prelude_string_get_string(out), prelude_string_get_len(out));
        prelude_string_destroy(out);

        return ret;
}



int idmef_criteria_to_string(idmef_criteria_t *criteria, prelude_string_t *out)
{
	if ( ! criteria )
		return -1;

	if ( criteria->or )
                prelude_string_sprintf(out, "((");

        idmef_criterion_to_string(criteria->criterion, out);
        
        if ( criteria->and ) {
                prelude_string_sprintf(out, " && ");
		idmef_criteria_to_string(criteria->and, out);
        }
        
        if ( criteria->or ) {
                prelude_string_sprintf(out, ") || (");
		idmef_criteria_to_string(criteria->or, out);
                prelude_string_sprintf(out, "))");
        }
        
	return 0;
}



prelude_bool_t idmef_criteria_is_criterion(idmef_criteria_t *criteria)
{
	return (criteria->criterion != NULL) ? TRUE : FALSE;
}



idmef_criterion_t *idmef_criteria_get_criterion(idmef_criteria_t *criteria)
{
	return criteria->criterion;
}



void idmef_criteria_or_criteria(idmef_criteria_t *criteria, idmef_criteria_t *criteria2)
{
        idmef_criteria_t *last = NULL;
        
        while ( criteria ) {
                last = criteria;
                criteria = criteria->or;
        }
        
        last->or = criteria2;
}



int idmef_criteria_and_criteria(idmef_criteria_t *criteria, idmef_criteria_t *criteria2)
{
        int ret;
        idmef_criteria_t *new, *last = NULL;
        
        while ( criteria ) {
                last = criteria;
                
                if ( criteria->or ) {
                        ret = idmef_criteria_clone(criteria2, &new);
                        if ( ret < 0 )
                                return ret;
                        
                        ret = idmef_criteria_and_criteria(criteria->or, new);
                        if ( ret < 0 )
                                return ret;
                }
                
                criteria = criteria->and;
        }
        
        last->and = criteria2;

        return 0;
}




void idmef_criteria_set_criterion(idmef_criteria_t *criteria, idmef_criterion_t *criterion)
{
        criteria->criterion = criterion;
}



int idmef_criteria_match(idmef_criteria_t *criteria, idmef_message_t *message)
{
        int ret;
        idmef_criteria_t *next;
        
        ret = idmef_criterion_match(criteria->criterion, message);
        if ( ret < 0 )
                next = criteria->or;
        else
                next = criteria->and;
        
        if ( ! next )
                return ret;
                
        return idmef_criteria_match(next, message);
}
