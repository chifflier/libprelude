# Copyright (C) 2003,2004 Nicolas Delon <nicolas@prelude-ids.org>
# All Rights Reserved
#
# This file is part of the Prelude program.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

package GenerateIDMEFTreeWrapC;

use Generate;
@ISA = qw/Generate/;

use strict;
use IDMEFTree;

sub	header
{
    my	$self = shift;
    my	$file = $self->{file};

    $self->output("
/*****
*
* Copyright (C) 2001-2005 Yoann Vandoorselaere <yoann\@prelude-ids.org>
* Copyright (C) 2003,2004 Nicolas Delon <nicolas\@prelude-ids.org>
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

/* Auto-generated by the GenerateIDMEFTreeWrapC package */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>

#include \"prelude-inttypes.h\"
#include \"prelude-list.h\"
#include \"prelude-msg.h\"
#include \"prelude-string.h\"

#define PRELUDE_ERROR_SOURCE_DEFAULT PRELUDE_ERROR_SOURCE_IDMEF_TREE_WRAP
#include \"prelude-error.h\"

#include \"idmef-time.h\"
#include \"idmef-data.h\"
#include \"idmef-type.h\"
#include \"idmef-value.h\"

#include \"idmef-tree-wrap.h\"

#define LISTED_OBJECT(name, type) prelude_list_t name

#define IS_LISTED prelude_list_t list

#define	UNION(type, var) type var; union

#define	UNION_MEMBER(value, type, name) type name

#define ENUM(...) typedef enum

#define PRE_DECLARE(type, class)

#define TYPE_ID(type, id) type

#define PRIMITIVE_TYPE(type)
#define PRIMITIVE_TYPE_STRUCT(type)

#define HIDE(type, name) type name

#define REFCOUNT int refcount

#define DYNAMIC_IDENT(x) uint64_t x

#define OPTIONAL_INT(type, name) type name; int name ## _is_set:1

");
}

sub	struct_desc
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;
    my $line;

    $self->output("\n\nstruct idmef_$struct->{short_typename} \{");

    foreach ( @{ $struct->{desc} } ) {
	$line = $_;
	$line =~ s/.*TYPE_ID.*//;
        $line =~ s/struct {//;
    $self->output(" $line\n");

    }
    #$self->output(" $_\n") foreach ( @{ $struct->{desc} } ~ s/.*TYPE_ID.*// );
    $self->output("\};\n\n");
}

sub	struct_constructor
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;

    $self->output("
/**
 * idmef_$struct->{short_typename}_new:
 * \@ret: Pointer where to store the created #$struct->{typename} object.
 *
 * Create a new #$struct->{typename} object.
 *
 * Returns: 0 on success, a negative value if an error occured.
 */
int idmef_$struct->{short_typename}_new($struct->{typename} **ret)
\{
	*ret = calloc(1, sizeof(**ret));
	if ( ! *ret )
		return prelude_error_from_errno(errno);
");

    if ( $struct->{is_listed} ) {
	$self->output("
	prelude_list_init(&(*ret)->list);
");
    }

    if ( $struct->{refcount} ) {
	$self->output("
	(*ret)->refcount = 1;
");
    }

    foreach my $field ( map { $_->{metatype} & &METATYPE_LIST ? $_ : () } @{ $struct->{field_list} } ) {
	$self->output("
	prelude_list_init(&(*ret)->$field->{name});
    
");

    }
    $self->output("
	return 0;

\}
");
}

sub	struct_get_child
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;
    my	$n = 0;

    $self->output("
int idmef_$struct->{short_typename}_get_child(void *p, idmef_child_t child, void **childptr)
\{
	$struct->{typename} *ptr = p;
	
	switch ( child ) \{
");

    foreach my $field ( @{ $struct->{field_list} } ) {

	if ( $field->{metatype} & &METATYPE_LIST ) {

	    $self->output("
		case $n:
                        *childptr = &ptr->$field->{name};
			return 0;
");
	} elsif ( $field->{metatype} & &METATYPE_UNION ) {

	    $self->output("
		case ${n}:
			return idmef_$struct->{short_typename}_get_$field->{var}_value(ptr, (idmef_value_t **) childptr);
");

	    $n++;

	    foreach my $member ( @{ $field->{member_list} } ) {
		$self->output("
		case $n:
                        *childptr = ( ptr->$field->{var} == $member->{value} ) ? ptr->$field->{name}.$member->{name} : NULL;
			return 0;   	   
");
		$n++;
	    }
	} elsif ( $field->{metatype} & (&METATYPE_PRIMITIVE | &METATYPE_ENUM) ) {

	    $self->output("
		case $n:
			return idmef_$struct->{short_typename}_get_$field->{short_name}_value(ptr, (idmef_value_t **) childptr);

");
	} else {
	    my $refer = $field->{ptr} ? "" : "&";

	    $self->output("
		case ${n}:
                        *childptr = ${refer}ptr->$field->{name};
                        return 0;
");
	}

	$n++;
    }

    $self->output("
		default:
			return prelude_error(PRELUDE_ERROR_IDMEF_TYPE_UNKNOWN_CHILD);
	\}
\}
");
}

sub	struct_new_child
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;
    my	$n = 0;

    $self->output("
int idmef_$struct->{short_typename}_new_child(void *p, idmef_child_t child, int n, void **ret)
\{
	$struct->{typename} *ptr = p;
    
	switch ( child ) \{
");

    foreach my $field ( @{ $struct->{field_list} } ) {

	if ( $field->{metatype} & &METATYPE_LIST ) {
	    $self->output("
		case $n: \{
                        int i, j;
                        int retval;
			prelude_list_t *tmp;

			if ( n < 0 ) \{
				 /* n < 0 denotes that we just have to add a list element */
				
				 retval = idmef_$struct->{short_typename}_new_$field->{short_name}(ptr, ($field->{typename} **) ret);
			\} else \{
				/* n >= 0, so the new element has to be n-th in the list  */
			
				/* get n-th element of the list */
				i = 0;
				prelude_list_for_each(&ptr->$field->{name}, tmp) \{
			    		*ret = prelude_list_entry(tmp, $field->{typename}, list);
					if ( i++ == n )
						return 0;
				
				\}
			
				/* we must add n-i list elements */
				for ( j = i; j <= n; j++ ) \{
			    		retval = idmef_$struct->{short_typename}_new_$field->{short_name}(ptr, ($field->{typename} **) ret);
					if ( retval < 0 )
						return retval;
				\}
			\}
			
			return 0;
		\}
");
	} elsif ( $field->{metatype} & &METATYPE_UNION ) {

	    $n++;

	    foreach my $member ( @{ $field->{member_list} } ) {
		$self->output("
		case $n:
			return idmef_$struct->{short_typename}_new_$member->{name}(ptr, ($member->{typename} **) ret);
");
		$n++;
	    }
	    
	} elsif ( $field->{metatype} & &METATYPE_NORMAL ) {
	    $self->output("
		case $n:
			return idmef_$struct->{short_typename}_new_$field->{name}(ptr, ($field->{typename} **) ret);
");
	}
	
	$n++;
    }

    $self->output("
		default:
			return prelude_error(PRELUDE_ERROR_IDMEF_TYPE_UNKNOWN_CHILD);
	\}
\}
");
}

sub	struct_destroy_internal
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;

    $self->output("
static void idmef_$struct->{short_typename}_destroy_internal($struct->{typename} *ptr)
\{
");

    $self->output("
       if ( ! prelude_list_is_empty(&ptr->list) )
               prelude_list_del_init(&ptr->list);
    ") if ( $struct->{is_listed} );

    foreach my $field ( @{ $struct->{field_list} } ) {
	my $destroy_func = "$field->{short_typename}_destroy";
	my $destroy_internal_func = "${destroy_func}_internal";

	if ( ! ($field->{metatype} & &METATYPE_PRIMITIVE) ) {
	    $destroy_func = "idmef_${destroy_func}";
	    $destroy_internal_func = "idmef_$destroy_internal_func";
	}
	
	if ( $field->{metatype} & &METATYPE_LIST ) {

	    $self->output("
	\{
		prelude_list_t *n, *tmp;
		$field->{typename} *entry;

		prelude_list_for_each_safe(&ptr->$field->{name}, tmp, n) \{
			entry = prelude_list_entry(tmp, $field->{typename}, list);
			$destroy_func(entry);
		\}
	\}
");

	} elsif ( $field->{metatype} & &METATYPE_UNION ) {
	    $self->output("
	switch ( ptr->$field->{var} ) {
");

	    foreach my $member ( @{ $field->{member_list} } ) {

		$self->output("
		case $member->{value}:
			idmef_$member->{short_typename}_destroy(ptr->$field->{name}.$member->{name});
			ptr->$field->{name}.$member->{name} = NULL;
			break;
");
	    }
	    $self->output("
		default:
			break;
	}
");
	} elsif ( $field->{metatype} & &METATYPE_STRUCT ) {
	    if ( $field->{ptr} ) {
		$self->output("
	if ( ptr->$field->{name} ) \{
		${destroy_func}(ptr->$field->{name});
		ptr->$field->{name} = NULL;
	\}
");
	    } else {
		$self->output("
	${destroy_internal_func}(&ptr->$field->{name});
");
	    }
	} elsif ( $field->{metatype} & &METATYPE_PRIMITIVE ) {

	    if ( $field->{ptr} ) {
		$self->output("
	if ( ptr->$field->{name} )
		free(ptr->$field->{name});
");
	    }
	}
    }

    $self->output("

	/* free() should be done by the caller */
\}
");
}

sub	struct_destroy
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;

    return if ( $struct->{toplevel} );

    $self->output("
/**
 * idmef_$struct->{short_typename}_destroy:
 * \@ptr: pointer to a #$struct->{typename} object.
 * 
 * Destroy \@ptr and all of it's children.
 * The objects are only destroyed if their reference count reach zero.
 */
");

    if ( $struct->{refcount} ) {
	$self->output("
void idmef_$struct->{short_typename}_destroy($struct->{typename} *ptr)
\{
	if ( --ptr->refcount )
		return;

	idmef_$struct->{short_typename}_destroy_internal(ptr);
        free(ptr);
\}
");

    } else {
	$self->output("
void idmef_$struct->{short_typename}_destroy($struct->{typename} *ptr)
\{
	idmef_$struct->{short_typename}_destroy_internal(ptr);
        free(ptr);
\}
");
    }
}

sub	struct_ref
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;

    $struct->{refcount} or return;

    $self->output("
/**
 * idmef_$struct->{short_typename}_ref:
 * \@ptr: pointer to a #$struct->{typename} object.
 *
 * Increase \@ptr reference count, so that it can be referenced
 * multiple time.
 *
 * Returns: a pointer to \@ptr.
 */
$struct->{typename} *idmef_$struct->{short_typename}_ref($struct->{typename} *ptr)
\{
	ptr->refcount++;

	return ptr;
\}
");
}

sub	struct_field_normal
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;
    my	$field = shift;
    my	$name = shift || $field->{name};
    my	$ptr = "";
    my	$refer = "";

    if ( $field->{ptr} ) {
	if ( $field->{metatype} & &METATYPE_STRUCT ) {
	    $ptr = "*";

	} else {
	    $refer = "*";
	}

    } else {
	if ( $field->{metatype} & (&METATYPE_STRUCT|&METATYPE_OPTIONAL_INT) ) {
	    $ptr = "*";
	    $refer = "&";
	}
    }

    ##############################
    # Generate *_get_* functions #
    ##############################

    $self->output("
/**
 * ${ptr}idmef_$struct->{short_typename}_get_${name}:
 * \@ptr: pointer to a #$struct->{typename} object.
 *
 * Get ${name} children of the #$struct->{typename} object.
 *
 * Returns: a pointer to a $field->{typename} object, or NULL if the children object is not set.
 */
$field->{typename} ${ptr}idmef_$struct->{short_typename}_get_${name}($struct->{typename} *ptr)
\{");

    if ( $field->{metatype} & &METATYPE_OPTIONAL_INT ) {
	$self->output("
	return ptr->$field->{name}_is_set ? &ptr->$field->{name} : NULL;");
	
    } elsif ( $field->{metatype} & &METATYPE_PRIMITIVE && !($field->{metatype} & &METATYPE_STRUCT) && $field->{ptr}) {
	$self->output("
	return ptr->$field->{name} ? *ptr->$field->{name} : ($field->{typename}) 0;");

    } else {
	$self->output("
	return ${refer}ptr->$field->{name};");
    }

    $self->output("
\}
");
    
    
    ####################################
    # Generate *_get_*_value functions #
    ####################################

    if ( $field->{metatype} & &METATYPE_PRIMITIVE ) {
	$self->output("
int idmef_$struct->{short_typename}_get_${name}_value($struct->{typename} *ptr, idmef_value_t **value)
\{");
	if ( $field->{metatype} & &METATYPE_OPTIONAL_INT ) {
	    $self->output("
        if ( ! ptr->$field->{name}_is_set ) {
                *value = NULL;
                return 0;
        }

	return idmef_value_new_$field->{value_type}(value, ptr->$field->{name});
");
	    
	} else {
	    if ( $field->{ptr} ) {
		$self->output("
	if ( ! ptr->$field->{name} ) {
                *value = NULL;
		return 0;
        }
");
	    }

	    $self->output("
        int ret;

	ret = idmef_value_new_$field->{value_type}(value, ${refer}ptr->$field->{name});
	if ( ret < 0 )
		return ret;

	idmef_value_dont_have_own_data(*value);

	return 0;");
	}

	$self->output("
\}
");

    } elsif ( $field->{metatype} & &METATYPE_STRUCT ) {
	$self->output("
int idmef_$struct->{short_typename}_get_${name}_value($struct->{typename} *ptr, idmef_value_t **value)
\{");

	if ( $field->{ptr} ) {
	    $self->output("
	if ( ! ptr->$field->{name} ) {
		*value = NULL;
                return 0;
        }
");
	}

	$self->output("
	return idmef_value_new_object(value, IDMEF_OBJECT_TYPE_" . uc("$field->{short_typename}") . ", ${refer}ptr->$field->{name});
\}
");

    } elsif ( $field->{metatype} & &METATYPE_ENUM ) {
	my $value_func = "idmef_value_new_enum_from_numeric(value, IDMEF_OBJECT_TYPE_" . uc("$field->{short_typename}") . ", ptr->$field->{name})";

	$self->output("
int idmef_$struct->{short_typename}_get_${name}_value($struct->{typename} *ptr, idmef_value_t **value)
\{");

	if ( $field->{metatype} & &METATYPE_OPTIONAL_INT ) {
	    $self->output("
        if ( ! ptr->$field->{name}_is_set ) \{
              *value = NULL;
              return 0;
        \}

	return $value_func;");
	} else {
	    $self->output("
	return $value_func;
");
	}

	$self->output("
\}
");
    }
       

    ##############################
    # Generate *_set_* functions #
    ##############################


    $self->output("
/**
 * idmef_$struct->{short_typename}_set_$field->{name}:
 * \@ptr: pointer to a #$struct->{typename} object.
 * \@$field->{name}: pointer to a #$field->{typename} object.
 *
 * Set \@$field->{name} object as a children of \@ptr.
 * if \@ptr already contain an \@$field->{name} object, then it is destroyed,
 * and updated to point to the provided \@$field->{name} object.
 */
");

    if ( $field->{metatype} & &METATYPE_OPTIONAL_INT ) {
	$self->output("
void idmef_$struct->{short_typename}_set_$field->{name}($struct->{typename} *ptr, $field->{typename} $field->{name})
\{
	ptr->$field->{name} = $field->{name};
	ptr->$field->{name}_is_set = 1;
\}


void idmef_$struct->{short_typename}_unset_$field->{name}($struct->{typename} *ptr)
\{
        ptr->$field->{name}_is_set = 0;
\}

");

    } elsif ( $field->{metatype} & &METATYPE_STRUCT ) {
	if ( $field->{ptr} ) {
	    my $destroy_func = "$field->{short_typename}_destroy";

	    $destroy_func = "idmef_${destroy_func}" if ( ! ($field->{metatype} & &METATYPE_PRIMITIVE) );

	    $self->output("
void idmef_$struct->{short_typename}_set_$field->{name}($struct->{typename} *ptr, $field->{typename} *$field->{name})
\{
	if ( ptr->$field->{name} )
		${destroy_func}(ptr->$field->{name});

	ptr->$field->{name} = $field->{name};
\}
");
	} else {
	    my $destroy_internal_func = "$field->{short_typename}_destroy_internal";

	    $destroy_internal_func = "idmef_${destroy_internal_func}" if ( ! ($field->{metatype} & &METATYPE_PRIMITIVE) );

	    $self->output("
void idmef_$struct->{short_typename}_set_$field->{name}($struct->{typename} *ptr, $field->{typename} *$field->{name})
\{
	${destroy_internal_func}(&ptr->$field->{name});
	memcpy(&ptr->$field->{name}, $field->{name}, sizeof (ptr->$field->{name}));
	free($field->{name});
\}
");
	}
    } else {
	if ( $field->{ptr} ) {
	    $self->output("
void idmef_$struct->{short_typename}_set_$field->{name}($struct->{typename} *ptr, $field->{typename} $field->{name})
\{
	if ( ptr->$field->{name} )
		free(ptr->$field->{name});

	ptr->$field->{name} = malloc(sizeof (*ptr->$field->{name}));
	*ptr->$field->{name} = $field->{name};
\}
");

	} else {
	    $self->output("
void idmef_$struct->{short_typename}_set_$field->{name}($struct->{typename} *ptr, $field->{typename} $field->{name})
\{
	ptr->$field->{name} = $field->{name};
\}
");
	}
    }

    ##############################
    # Generate *_new_* functions #
    ##############################

    $self->output("
/**
 * idmef_$struct->{short_typename}_new_${name}:
 * \@ptr: pointer to a #$struct->{typename} object.
 * \@ret: pointer to an address where to store the created #$field->{typename} object.
 *
 * Create a new ${name} object, children of #$struct->{typename}.
 * If \@ptr already contain a #$field->{typename} object, then it is destroyed.
 *
 * Returns: 0 on success, or a negative value if an error occured.
 */
int idmef_$struct->{short_typename}_new_${name}($struct->{typename} *ptr, $field->{typename} **ret)
\{");
    if ( $field->{metatype} & &METATYPE_OPTIONAL_INT ) {
	$self->output("
	ptr->$field->{name}_is_set = 1;
");
    } elsif ( $field->{metatype} & &METATYPE_PRIMITIVE ) {

	if ( $field->{metatype} & &METATYPE_STRUCT ) {

	    if ( $field->{ptr} ) {

		$self->output("
        int retval;

	if ( ptr->$field->{name} )
		$field->{short_typename}_destroy(ptr->$field->{name});
		
	retval = $field->{short_typename}_new(&ptr->$field->{name});
        if ( retval < 0 )
               return retval;
");
	    } else {

		$self->output("
	$field->{short_typename}_destroy_internal(&ptr->$field->{name});
");
	    }

	} else {
	    if ( $field->{ptr} ) {
		$self->output("
	if ( ptr->$field->{name} )
		free(ptr->$field->{name});

	ptr->$field->{name} = calloc(1, sizeof (*ptr->$field->{name}));
");
	    }
	}

    } else {
	if ( $field->{metatype} & &METATYPE_STRUCT && $field->{ptr} ) {
	    $self->output("
        int retval;

	if ( ! ptr->$field->{name} )
		retval = idmef_$field->{short_typename}_new(&ptr->$field->{name});
");
	}
    }

    $refer = $field->{ptr} ? "" : "&";

    $self->output("
        *ret = ${refer}ptr->$field->{name};
	return 0;
\}
");
}

sub	struct_field_union
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;
    my	$field = shift;

    $self->output("
/** 
 * idmef_$struct->{short_typename}_get_$field->{var}:
 * \@ptr: pointer to a #$struct->{typename} object.
 *
 * Access the $field->{var} children of \@ptr.
 *
 * Returns: a pointer to the #$field->{typename} children, or NULL if it is not set.
 */
$field->{typename} idmef_$struct->{short_typename}_get_$field->{var}($struct->{typename} *ptr)
\{
	return ptr->$field->{var};
\}
");

    $self->output("
int idmef_$struct->{short_typename}_get_$field->{var}_value($struct->{typename} *ptr, idmef_value_t **value)
\{
	return idmef_value_new_enum_from_numeric(value, IDMEF_OBJECT_TYPE_" . uc("$field->{short_typename}") . ", ptr->$field->{var});
\}
");

    foreach my $member ( @{ $field->{member_list} } ) {
	$self->output("
/** 
 * idmef_$struct->{short_typename}_get_$member->{name}:
 * \@ptr: pointer to a #$struct->{typename} object.
 *
 * Access the $member->{name} children of \@ptr.
 *
 * Returns: a pointer to the #$member->{typename} children, or NULL if it is not set.
 */
$member->{typename} *idmef_$struct->{short_typename}_get_$member->{name}($struct->{typename} *ptr)
\{
	return (ptr->$field->{var} == $member->{value}) ? ptr->$field->{name}.$member->{name} : NULL;
\}
"
);

	$self->output("
/**
 * idmef_$struct->{short_typename}_set_$member->{name}:
 * \@ptr: pointer to a #$struct->{typename} object.
 * \@$member->{name}: pointer to a #$member->{typename} object.
 *
 * Set \@$member->{name} object as a children of \@ptr.
 * if \@ptr already contain a \@$member->{name} object, then it is destroyed,
 * and updated to point to the provided \@$member->{name} object.
 */
void idmef_$struct->{short_typename}_set_$member->{name}($struct->{typename} *ptr, $member->{typename} *$member->{name})
\{
	switch ( ptr->$field->{var} ) \{
");
	foreach my $member ( @{ $field->{member_list} } ) {
	    $self->output("
		case $member->{value}:
			idmef_$member->{short_typename}_destroy(ptr->$field->{name}.$member->{name});
			break;
");
	}

	$self->output("
		default:
			break;
	\}

	ptr->$field->{name}.$member->{name} = $member->{name};
	ptr->$field->{var} = $member->{value};
\}
");
	

	$self->output("
int idmef_$struct->{short_typename}_get_$member->{name}_value($struct->{typename} *ptr, idmef_value_t **value)
\{
        if ( ptr->$field->{var} != $member->{value} ) {
                *value = NULL;
                return 0;
        }

	return idmef_value_new_object(value, IDMEF_OBJECT_TYPE_" . uc("$member->{short_typename}") . ",
                                      ptr->$field->{name}.$member->{name});                        
\}
");

	$self->output("
/**
 * idmef_$struct->{short_typename}_new_$member->{name}:
 * \@ptr: pointer to a #$struct->{typename} object.
 * \@ret: pointer where to store the created #$member->{typename} object.
 *
 * Create a new $member->{typename} object, children of #$struct->{typename}.
 * If \@ptr already contain a #$member->{typename} object, then it is destroyed.
 *
 * Returns: 0 on success, or a negative value if an error occured.
 */
int idmef_$struct->{short_typename}_new_$member->{name}($struct->{typename} *ptr, $member->{typename} **ret)
\{
        int retval;

	switch ( ptr->$field->{var} ) \{
");
	foreach my $other_member ( @{ $field->{member_list} } ) {

	    if ( $other_member == $member ) {
		$self->output("
		case $member->{value}:
                        *ret = ptr->$field->{name}.$member->{name};
			return 0;
"); 
	    } else {
		$self->output("
		case $other_member->{value}:
			idmef_$other_member->{short_typename}_destroy(ptr->$field->{name}.$other_member->{name});
			break;
");
	    }
	}

	$self->output("
		default:
			break;
	\}

        retval = idmef_$member->{short_typename}_new(ret);
        if ( retval < 0 )
                return retval;

	ptr->$field->{var} = $member->{value};
        ptr->$field->{name}.$member->{name} = *ret;

	return 0;
\}
");
    }
}

sub	struct_field_list
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;
    my	$field = shift;
    my	$new_field_function = "$field->{short_typename}_new(ret)";

    $new_field_function = "idmef_${new_field_function}" if ( ! ($field->{metatype} & &METATYPE_PRIMITIVE) );

    $self->output("
/**
 * idmef_$struct->{short_typename}_get_next_$field->{short_name}:
 * \@ptr: pointer to a #$struct->{typename} object.
 * \@object: pointer to a #$field->{typename} object.
 *
 * Get the next #$field->{typename} object listed in \@ptr.
 * When iterating over the $field->{typename} object listed in \@ptr,
 * \@object should be set to the latest returned #$field->{typename} object.
 * 
 * Returns: the next #$field->{typename} in the list.
 */
$field->{typename} *idmef_$struct->{short_typename}_get_next_$field->{short_name}($struct->{typename} *ptr, $field->{typename} *object)
\{
        prelude_list_t *tmp = (object) ? &object->list : NULL;

        prelude_list_for_each_continue(&ptr->$field->{name}, tmp)
                return prelude_list_entry(tmp, $field->{typename}, list);

        return NULL;
\}

/**
 * idmef_$struct->{short_typename}_set_$field->{short_name}:
 * \@ptr: pointer to a #$struct->{typename} object.
 * \@object: pointer to a #$field->{typename} object.
 *
 * Add \@object to the tail of \@ptr list of #$field->{typename} object.
 */
void idmef_$struct->{short_typename}_set_$field->{short_name}($struct->{typename} *ptr, $field->{typename} *object)
\{
	prelude_list_add_tail(&ptr->$field->{name}, &object->list);
\}

/**
 * idmef_$struct->{short_typename}_new_$field->{short_name}:
 * \@ptr: pointer to a #$struct->{typename} object.
 * \@ret: pointer to an address where to store the created #$field->{typename} object.
 *
 * Create a new #$field->{typename} children of \@ptr,
 * and add it to the tail of \@ptr list of #$field->{typename} object.
 * 
 * Returns: 0 on success, or a negative value if an error occured.
 */
int idmef_$struct->{short_typename}_new_$field->{short_name}($struct->{typename} *ptr, $field->{typename} **ret)
\{	
        int retval;

	retval = $new_field_function;
	if ( retval < 0 )
		return retval;
	
	prelude_list_add_tail(&ptr->$field->{name}, &(*ret)->list);
	
	return 0;
\}


int idmef_$struct->{short_typename}_get_$field->{short_name}_value($struct->{typename} *ptr, idmef_value_t **value)
\{
        int ret;
	idmef_value_t *val;
        prelude_list_t *tmp;
	$field->{typename} *entry;
	
	ret = idmef_value_new_list(value);
	if ( ret < 0 )
		return ret;
	
	prelude_list_for_each(&ptr->$field->{name}, tmp) \{
		entry = prelude_list_entry(tmp, $field->{typename}, list);
");

    if ( $field->{metatype} & &METATYPE_PRIMITIVE ) {
	$self->output("
		ret = idmef_value_new_$field->{value_type}(&val, entry);
		if ( ret < 0 ) \{
			idmef_value_destroy(*value);
			return ret;
		\}
		idmef_value_dont_have_own_data(val);
");

    } else {
	$self->output("
		ret = idmef_value_new_object(&val, IDMEF_OBJECT_TYPE_" . uc("$field->{short_typename}") . ", entry);
		if ( ret < 0 ) \{
			idmef_value_destroy(*value);
			return ret;
		\}
");
    }

    $self->output("
                ret = idmef_value_list_add(*value, val);
		if ( ret < 0 ) \{
			idmef_value_destroy(*value);
			idmef_value_destroy(val);
			return ret;
		\}
	\}
	
	return 0;
\}

");
}

sub	struct_fields
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;

    foreach my $field ( @{ $struct->{field_list} } ) {
	$self->struct_field_normal($tree, $struct, $field) if ( $field->{metatype} & &METATYPE_NORMAL );
 	$self->struct_field_list($tree, $struct, $field) if ( $field->{metatype} & &METATYPE_LIST );
 	$self->struct_field_union($tree, $struct, $field) if ( $field->{metatype} & &METATYPE_UNION );
    }
}

sub	struct
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;

    $self->struct_desc($tree, $struct);
}

sub	struct_func
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;

    $self->struct_constructor($tree, $struct);
    $self->struct_ref($tree, $struct);
    $self->struct_get_child($tree, $struct);
    $self->struct_new_child($tree, $struct);
    $self->struct_destroy_internal($tree, $struct);
    $self->struct_destroy($tree, $struct);
    $self->struct_fields($tree, $struct);
}

sub	enum
{
    my	$self = shift;
    my	$tree = shift;
    my	$enum = shift;

    $self->output("
/**
 * idmef_$enum->{short_typename}_to_numeric:
 * \@name: pointer to an IDMEF string representation of a #$enum->{typename} value.
 *
 * Returns: the numeric equivalent of \@name, or -1 if \@name is not valid.
 */
$enum->{typename} idmef_$enum->{short_typename}_to_numeric(const char *name)
\{
");

    foreach my $field ( @{ $enum->{field_list} } ) {
	my $fullname = 'IDMEF_' . uc($enum->{short_typename});
	my $fieldname = $field->{name};
	$fieldname =~ s/^${fullname}_//;
	$fieldname = lc($fieldname);
	$fieldname =~ tr/_/-/;

	$self->output("
	if ( strcasecmp(name, \"$fieldname\" ) == 0)
		return $field->{name};
");
    }

    $self->output("
	return prelude_error(PRELUDE_ERROR_IDMEF_UNKNOWN_ENUM_STRING);
\}	
");

        $self->output("
/**
 * idmef_$enum->{short_typename}_to_string:
 * \@val: an enumeration value for #$enum->{typename}.
 *
 * Return the IDMEF string equivalent of \@val provided #$enum->{typename} value.
 *
 * Returns: a pointer to the string describing \@val, or NULL if \@val is invalid.
 */
const char *idmef_$enum->{short_typename}_to_string($enum->{typename} val)
\{
	switch ( val ) \{
");

    my $cnt = 0;

    foreach my $field ( @{ $enum->{field_list} } ) {

	if ( $cnt == 0 && $field->{value} != 0 ) {
	    $self->output("
		case 0:
			return NULL;
");
	}
	
	my $fullname = 'IDMEF_' . uc($enum->{short_typename});
	my $fieldname = $field->{name};
	$fieldname =~ s/^${fullname}_//;
	$fieldname = lc($fieldname);
	$fieldname =~ tr/_/-/;

	$self->output("
		case $field->{name}:
			return \"$fieldname\";
");

	$cnt++;
    }


    $self->output("
		default:
			return NULL;
	\}

	return NULL;
\}
");
}


sub footer 
{
    my $self = shift;

    $self->output("
void idmef_message_set_pmsg(idmef_message_t *message, prelude_msg_t *msg)
\{
        message->pmsg = msg;
\}


prelude_msg_t *idmef_message_get_pmsg(idmef_message_t *message)
\{
        return message->pmsg;
\}


/**
 * idmef_message_destroy:
 * \@ptr: pointer to a #idmef_message_t object.
 *
 * Destroy \@ptr and all of it's children.
 * The objects are only destroyed if their reference count reach zero.
 */
void idmef_message_destroy(idmef_message_t *ptr)
\{
        if ( --ptr->refcount )
                return;

        idmef_message_destroy_internal(ptr);

        if ( ptr->pmsg )
                prelude_msg_destroy(ptr->pmsg);

        free(ptr);
\}
");

}
1;
