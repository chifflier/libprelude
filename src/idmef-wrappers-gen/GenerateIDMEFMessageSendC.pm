# Copyright (C) 2003 Nicolas Delon <delon.nicolas@wanadoo.fr>
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

package GenerateIDMEFMessageSendC;

use Generate;
@ISA = qw/Generate/;

use strict;
use IDMEFTree;

sub	header
{
     my	$self = shift;
     my	$time = localtime;

     $self->output("
/* Auto-generated by the GenerateIDMEFMessageSendC package ($time) */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include \"list.h\"
#include \"prelude-log.h\"
#include \"prelude-io.h\"
#include \"prelude-message.h\"
#include \"prelude-message-buffered.h\"
#include \"prelude-message-id.h\"
#include \"idmef-message-id.h\"
#include \"idmef.h\"
#include \"idmef-tree-wrap.h\"
#include \"idmef-message-send.h\"
#include \"sensor.h\"
#include \"prelude-client.h\"
#include \"common.h\"

/*
 * If you wonder why we do this, and why life is complicated,
 * then wonder why the hell the guys that wrote IDMEF choose to use XML.
 * XML is dog slow. And XML'll never achieve performance needed for real time IDS.
 *
 * Here we are trying to communicate using a home made, binary version of IDMEF.
 */


static inline void idmef_send_string(prelude_msgbuf_t *msg, uint8_t tag, idmef_string_t *string)
\{
        if ( ! string || ! idmef_string_get_string(string) )
                return;

        prelude_msgbuf_set(msg, tag, idmef_string_get_len(string), idmef_string_get_string(string));
\}



static inline void idmef_send_uint64(prelude_msgbuf_t *msg, uint8_t tag, uint64_t data) 
\{
        uint64_t dst;
        
        if ( data == 0 )
                return;

        dst = prelude_hton64(data);
        
        prelude_msgbuf_set(msg, tag, sizeof(dst), &dst);
\}



static inline void idmef_send_uint32(prelude_msgbuf_t *msg, uint8_t tag, uint32_t data) 
\{        
        if ( data == 0 )
                return;
        
        data = htonl(data);
        prelude_msgbuf_set(msg, tag, sizeof(data), &data);
\}



static inline void idmef_send_uint16(prelude_msgbuf_t *msg, uint8_t tag, uint16_t data) 
\{
        if ( data == 0 )
                return;
        
        data = htons(data);
        prelude_msgbuf_set(msg, tag, sizeof(data), &data);
\}



static inline void idmef_send_float(prelude_msgbuf_t *msg, uint8_t tag, float data)
\{
	if ( data == 0.0 )
		return;

	prelude_msgbuf_set(msg, tag, sizeof (data), &data);
\}



inline void idmef_send_time(prelude_msgbuf_t *msg, uint8_t tag, idmef_time_t *data) 
\{
	idmef_time_t dst;

	if ( ! data)
		return;

	idmef_time_set_sec(&dst, htonl(idmef_time_get_sec(data)));
	idmef_time_set_usec(&dst, htonl(idmef_time_get_usec(data)));

	prelude_msgbuf_set(msg, tag, sizeof (dst), &dst);
\}



static inline void idmef_send_data(prelude_msgbuf_t *msg, uint8_t tag, idmef_data_t *data)
\{
	if ( ! data )
		return;

	prelude_msgbuf_set(msg, tag, idmef_data_get_len(data), idmef_data_get_data(data));
\}

");
}

sub	struct_field_normal
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;
    my	$field = shift;
    my	$type = shift || $field->{short_typename};

    $self->output(" " x 8, 
		  "idmef_send_${type}(msg, ", 
		  "MSG_",  uc($struct->{short_typename}), "_", uc($field->{name}),
		  ", idmef_$struct->{short_typename}_get_$field->{name}($struct->{short_typename}));\n");
}

sub	struct_field_struct
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;
    my	$field = shift;

    $self->output(" " x 8, 
		  "idmef_send_$field->{short_typename}(msg, idmef_$struct->{short_typename}_get_$field->{name}($struct->{short_typename}));",
		  "\n");
}

sub	struct_field_list
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;
    my	$field = shift;

    $self->output("\n");
    $self->output(" " x 8, "{\n");
    $self->output(" " x 16, "$field->{typename} *$field->{short_name} = NULL;", "\n\n");
    $self->output(" " x 16, "while ( ($field->{short_name} = idmef_$struct->{short_typename}_get_next_$field->{short_name}($struct->{short_typename}, $field->{short_name})) ) {", "\n");

    if ( $field->{metatype} & &METATYPE_PRIMITIVE ) {
	$self->output(" " x 24,
		      "idmef_send_$field->{short_typename}(msg, ",
		      "MSG_", uc($struct->{short_typename}), "_", uc($field->{short_name}),
		      ", $field->{short_name});\n");

    } else {
	$self->output(" " x 24, "idmef_send_$field->{short_typename}(msg, $field->{short_name});", "\n");
    }

    $self->output(" " x 16, "}\n");
    $self->output(" " x 8, "}\n");
    $self->output("\n");
}

sub	struct_field_union
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;
    my	$field = shift;

    $self->output("\n");
    $self->output(" " x 8, "switch ( idmef_$struct->{short_typename}_get_$field->{var}($struct->{short_typename}) ) {", "\n\n");

    foreach my $member ( @{$field->{member_list}} ) {
	$self->output(" " x 16, "case $member->{value}:", "\n");
	$self->output(" " x 24, "idmef_send_$member->{short_typename}(msg, idmef_$struct->{short_typename}_get_$member->{name}($struct->{short_typename}));", "\n");
	$self->output(" " x 24, "break;", "\n\n");
    }

    $self->output(" " x 16, "default:", "\n",
		  " " x 24, "/* nop */;", "\n\n");
    $self->output(" " x 8, "}\n");
}

sub	pre_declared
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;

    $self->output("void idmef_send_$struct->{short_typename}(prelude_msgbuf_t *, $struct->{typename} *);", "\n\n");
}

sub	struct
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;

    $self->output("void idmef_send_$struct->{short_typename}(prelude_msgbuf_t *msg, $struct->{typename} *$struct->{short_typename})\n\{\n");

    $self->output(" " x 8, "if ( ! $struct->{short_typename} )", "\n",
		  " " x 16, "return;",
		  "\n\n");

    if ( ! $struct->{toplevel} ) {
	$self->output(" " x 8, "prelude_msgbuf_set(msg, ", "MSG_" . uc($struct->{short_typename}) . "_TAG", ", 0, NULL);", "\n\n");
    }

    foreach my $field ( @{ $struct->{field_list} } ) {

	if ( $field->{metatype} & &METATYPE_NORMAL ) {

	    if ( $field->{metatype} & &METATYPE_PRIMITIVE ) {
		$self->struct_field_normal($tree, $struct, $field);

	    } elsif ( $field->{metatype} & &METATYPE_STRUCT ) {
		$self->struct_field_struct($tree, $struct, $field);

	    } elsif ( $field->{metatype} & &METATYPE_ENUM ) {
		$self->struct_field_normal($tree, $struct, $field, "uint32");
	    }

	} elsif ( $field->{metatype} & &METATYPE_LIST ) {
	    $self->struct_field_list($tree, $struct, $field);

	} elsif ( $field->{metatype} & &METATYPE_UNION ) {
	    $self->struct_field_union($tree, $struct, $field);
	}
    }

    if ( ! $struct->{toplevel} ) {
	$self->output("\n", " " x 8, "prelude_msgbuf_set(msg, MSG_END_OF_TAG, 0, NULL);", "\n");
    }

    $self->output("\}\n\n\n");
}

1;
