/*****
*
* Copyright (C) 2001, 2002, 2003, 2004 Yoann Vandoorselaere <yoann@prelude-ids.org>
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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

#include "common.h"
#include "prelude-client.h"
#include "prelude-message-id.h"
#include "prelude-log.h"
#include "prelude-io.h"
#include "prelude-auth.h"
#include "prelude-message.h"
#include "prelude-getopt.h"
#include "plaintext.h"
#include "ssl-register.h"
#include "config.h"
#include "client-ident.h"


static int gid_set = 0;
static int uid_set = 0;
static uint16_t port = 5553;
static const char *addr = NULL;
static prelude_client_t *client;



static int set_sensor_name(void **context, prelude_option_t *opt, const char *optarg) 
{
        prelude_client_set_name(client, optarg);
        return prelude_option_success;
}



static int set_sensor_uid(void **context, prelude_option_t *opt, const char *optarg) 
{
        uid_set = 1;
        prelude_client_set_uid(client, atoi(optarg));
        return prelude_option_success;
}



static int set_sensor_gid(void **context, prelude_option_t *opt, const char *optarg)
{
        gid_set = 1;
        prelude_client_set_gid(client, atoi(optarg));
        return prelude_option_success;
}



static int set_manager_addr(void **context, prelude_option_t *opt, const char *optarg) 
{
        char *ptr;
        
        addr = strdup(optarg);
        if ( ! addr ) {
                log(LOG_ERR, "couldn't duplicate string.\n");
                return prelude_option_error;
        }

        ptr = strchr(addr, ':');
        if ( ptr ) {
                *ptr++ = '\0';
                port = atoi(ptr);
        } 
        
        return prelude_option_success;
}



static int print_help(void **context, prelude_option_t *opt, const char *optarg) 
{
        prelude_option_print(NULL, CLI_HOOK, 50);
        return prelude_option_end;
}




static prelude_io_t *connect_manager(struct in_addr in, uint16_t port) 
{
        int ret, sock;
        prelude_io_t *fd;
        struct sockaddr_in daddr;

        daddr.sin_family = AF_INET;
        daddr.sin_port = htons(port);

        daddr.sin_addr = in;

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if ( sock < 0) {
                fprintf(stderr, "error creating socket.\n");
                return NULL;
        }

        ret = connect(sock, (struct sockaddr *) &daddr, sizeof(daddr));
        if ( ret < 0 ) {
                fprintf(stderr, "couldn't connect to %s.\n", addr);
                close(sock);
                return NULL;
        }

        fd = prelude_io_new();
        if ( ! fd ) {
                fprintf(stderr, "error creating an IO object.\n");
                close(sock);
                return NULL;
        }

        fprintf(stderr, "Succeeded.\n");
        
        prelude_io_set_sys_io(fd, sock);

        return fd;
}




/*
 * Report server should send us a message containing
 * information about the kind of connecition it support.
 */
static int get_manager_setup(prelude_io_t *fd, int *have_ssl, int *have_plaintext) 
{
        int ret;
        void *buf;
        uint8_t tag;
        uint32_t dlen;
        prelude_msg_t *msg = NULL;
        prelude_msg_status_t status;

        do {
                status = prelude_msg_read(&msg, fd);
        } while ( status == prelude_msg_unfinished );

        if ( status != prelude_msg_finished ) {
                log(LOG_ERR, "an error occured while reading the Manager configuration msg.\n");
                return -1;
        }
        
        if ( prelude_msg_get_tag(msg) != PRELUDE_MSG_AUTH ) {
                log(LOG_ERR, "Manager didn't sent us any authentication message.\n");
                return -1;
        }

        while ( (ret = prelude_msg_get(msg, &tag, &dlen, &buf)) > 0  ) {
                
                switch (tag) {

                case PRELUDE_MSG_AUTH_HAVE_SSL:
                        *have_ssl = 1;
                        break;

                case PRELUDE_MSG_AUTH_HAVE_PLAINTEXT:
                        *have_plaintext = 1;
                        break;

                default:
                        log(LOG_ERR, "Invalid authentication tag %d.\n", tag);
                        goto err;
                }
        }
        
 err:
        prelude_msg_destroy(msg);
        return ret;
}



static int ask_one_shot_password(char **buf) 
{
        int ret;
        char *pass, *confirm;

        fprintf(stderr,
                "\n\nPlease use the one-shot password provided by the \"manager-adduser\" program.\n\n");
        
        pass = getpass("Enter registration one shot password : ");
        if ( ! pass )
                return -1;

        pass = strdup(pass);
        if ( ! pass )
                return -1;
        
        confirm = getpass("Please confirm one shot password : ");
        if ( ! confirm )
                return -1;

        ret = strcmp(pass, confirm);
        memset(confirm, 0, strlen(confirm));

        if ( ret == 0 ) {
                *buf = pass;
                return 0;
        }

        fprintf(stderr, "Bad password, they don't match.\n");
        
        memset(pass, 0, strlen(pass));
        free(pass);
        
        return ask_one_shot_password(buf);
}




static int handle_argument(int argc, char **argv) 
{
        int ret;
        
        prelude_option_add(NULL, CLI_HOOK, 'h', "help", "Print this help",
                           no_argument, print_help, NULL);
        
        prelude_option_add(NULL, CLI_HOOK, 's', "sensorname", "Name of the sensor to register",
                           required_argument, set_sensor_name, NULL);
        
        prelude_option_add(NULL, CLI_HOOK, 'u', "uid", "Unix UID used by the sensor to register",
                           required_argument, set_sensor_uid, NULL);

        prelude_option_add(NULL, CLI_HOOK, 'g', "gid", "Unix GID used by the sensor to register",
                           required_argument, set_sensor_gid, NULL);
        
        prelude_option_add(NULL, CLI_HOOK, 'm', "manager-addr", "Address where the Prelude Manager is listening",
                           required_argument, set_manager_addr, NULL);
        
        ret = prelude_option_parse_arguments(NULL, NULL, NULL, argc, argv);
        if ( ret == prelude_option_end || ret == prelude_option_error )
                exit(ret);
        
        if ( ! addr ) {
                log(LOG_INFO, "No manager address specified.\n");
                return -1;
        }

        if ( ! prelude_client_get_name(client) ) {
                log(LOG_INFO, "No sensor name specified.\n");
                return -1;
        }

        if ( ! uid_set || ! gid_set ) {
                if ( ! uid_set )
                        prelude_client_set_uid(client, getuid());

                if ( ! gid_set )
                        prelude_client_set_gid(client, getgid());
                
                log(LOG_INFO,
                    "\n\n*** WARNING ***\n"
                    "sensor-adduser was started without the --uid command line option.\n\n"

                    "It means that sensor specific files, that should be available for writing,\n"
                    "will be created using the current UID which is %u.\n\n"

                    "Your sensor won't start unless it is running under this UID.\n\n"
                    "[Please press enter if this is what you plan to do]\n", prelude_client_get_uid(client));

                while ( getchar() != '\n' );
        }
        
        return 0;
}




/*
 * the elf_hash function was stolen from binutil :
 * binutils-2.12.90.0.7/bfd/elf.c:
 *
 * Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002
 * Free Software Foundation, Inc.
 */
static uint32_t elf_hash(const unsigned char *name)
{
        uint32_t h, g;
        unsigned char c;

        h = 0;
        while ( (c = *name++) != '\0' ) {

                h = (h << 4) + c;

                if ( (g = (h & 0xf0000000)) != 0 )
                        h ^= g >> 24;

                h &= ~g;
        }

        return h;
}



static uint32_t time_hash(void) 
{
        struct timeval tv;

        gettimeofday(&tv, NULL);
        
        return tv.tv_sec ^ tv.tv_usec;
}



static uint64_t generate_analyzerid(const char *hostname, const char *sensorname) 
{
        union {
                uint64_t val64;
                uint32_t val32[2];
        } combo;
        
        combo.val32[0] = time_hash();
        combo.val32[1] = elf_hash(hostname) ^ elf_hash(sensorname);
        
        return combo.val64;
}




static int register_sensor_ident(const char *sname, uint64_t *ident) 
{
        int ret;
        FILE *fd;
        char buf[256], *ptr = buf, *identptr, *name;
        
        fd = fopen(PRELUDE_IDENT_FILE, "a+");
        if ( ! fd ) {
                log(LOG_ERR, "error opening %s for writing.\n", PRELUDE_IDENT_FILE);
                return -1;
        }
        
        /*
         * make sure the file is readable by all.
         */
        ret = fchmod(fileno(fd), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if ( ret < 0 ) 
                log(LOG_ERR, "couldn't make ident file readable for all.\n");

        rewind(fd);
        
        while ( fgets(buf, sizeof(buf), fd) ) {

                name = strtok(ptr, ":");
                if ( ! name ) {
                        log(LOG_ERR, "malformed sensor identity file.\n");
                        return -1;
                }

                identptr = strtok(NULL, ":");
                if ( ! identptr ) {
                        log(LOG_ERR, "malformed sensor identity file.\n");
                        return -1;
                }

                if ( strcmp(name, sname) == 0 ) {
                        
                        /*
                         * sensor already have an ident.
                         */
                        sscanf(identptr, "%llu", ident);
                        fclose(fd);
                        
                        return 1;
                }
        }
        
        fprintf(fd, "%s:%llu\n", sname, *ident);
        fclose(fd);
        
        return ret;
}




static int setup_sensor_files(const char *hostname, uint64_t analyzerid) 
{
        int fd, ret;
        char buf[256];
        const char *sname;

        sname = prelude_client_get_name(client);
        prelude_client_get_backup_filename(client, buf, sizeof(buf));

        /*
         * The user may have changed permission, and we don't want
         * to be vulnerable to a symlink attack anyway. 
         */
        fd = prelude_open_persistant_tmpfile(buf, 0, S_IRUSR|S_IWUSR);
        if ( fd < 0 ) {
                log(LOG_ERR, "couldn't create %s.\n", buf);
                return -1;
        }

        ret = fchown(fd, prelude_client_get_uid(client), prelude_client_get_gid(client));
        if ( ret < 0 ) {
                log(LOG_ERR, "could not chown %s to UID %d GID %d.\n", buf,
                    prelude_client_get_uid(client), prelude_client_get_gid(client));
                return -1;
        }

        ret = register_sensor_ident(sname, &analyzerid);
        if ( ret == 0 )
                fprintf(stderr, "Allocated ident for %s@%s: %llu.\n", sname, hostname, analyzerid);

        else if ( ret == 1)
                fprintf(stderr, "Using already allocated ident for %s@%s: %llu.\n", sname, hostname, analyzerid);

        return 0;
}





int main(int argc, char **argv) 
{
        int ret;
        prelude_io_t *fd;
        struct in_addr in;
        uint64_t analyzerid;
        char buf[256], *pass;
        int have_ssl = 0, have_plaintext = 0;

        client = prelude_client_new(0);
        if ( ! client )
                return -1;
        
        ret = handle_argument(argc, argv);
        if ( ret < 0 )
                return -1;
        
        fprintf(stderr,
                "\n\nNow please start \"manager-adduser\" on the Manager host where\n"
                "you wish to add the new user.\n\n"

                "Please remember that you should call \"sensor-adduser\" for each configured\n"
                "Manager entry.\n\n"
                
                "Press enter when done.\n");

        fgets(buf, sizeof(buf), stdin);

        
        ret = ask_one_shot_password(&pass);
        if ( ret < 0 )
                return -1;
        
	fprintf(stderr, "connecting to Manager host (%s:%d)... ", addr, port);
        
        ret = prelude_resolve_addr(addr, &in);
        if ( ret < 0 ) {
                fprintf(stderr, "couldn't resolve %s.\n", addr);
                return -1;
        }
        
        fd = connect_manager(in, port);
        if ( ! fd ) 
                return -1;

        ret = get_manager_setup(fd, &have_ssl, &have_plaintext);
        if ( ret < 0 )
                return -1;

        gethostname(buf, sizeof(buf));
        analyzerid = generate_analyzerid(prelude_client_get_name(client), buf);
        
#ifdef HAVE_SSL
        if ( have_ssl && strcmp(inet_ntoa(in), "127.0.0.1") != 0 ) {
                ret = ssl_add_certificate(client, prelude_client_get_name(client),
                                          fd, pass, strlen(pass), prelude_client_get_uid(client));
                goto end;
        }
#endif

        if ( have_plaintext ) {
                ret = create_plaintext_user_account(client, fd, pass);
                goto end;
        } else {
                log(LOG_INFO, "couldn't agree on a protocol to use.\n");
                ret = -1;
        }

        exit(1);
        
 end:
        if ( ret < 0 )
                exit(ret);
        
        exit(setup_sensor_files(buf, analyzerid));
}






