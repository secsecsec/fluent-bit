/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2015-2017 Treasure Data Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <fluent-bit/flb_info.h>
#include <fluent-bit/flb_config.h>
#include <fluent-bit/flb_input.h>

#include <unistd.h>

#include "systemd_db.h"
#include "systemd_config.h"

struct flb_systemd_config *flb_systemd_config_create(struct flb_input_instance *i_ins,
                                                     struct flb_config *config)
{
    int ret;
    char *tmp;
    struct mk_list *head;
    struct flb_config_prop *prop;
    struct flb_systemd_config *ctx;

    /* Allocate space for the configuration */
    ctx = flb_calloc(1, sizeof(struct flb_systemd_config));
    if (!ctx) {
        flb_errno();
        return NULL;
    }

    /* Open the Journal */
    ret = sd_journal_open(&ctx->j, SD_JOURNAL_LOCAL_ONLY);
    if (ret != 0) {
        flb_free(ctx);
        flb_error("[in_systemd] could not open the Journal");
        return NULL;
    }
    ctx->fd = sd_journal_get_fd(ctx->j);
    ctx->i_ins = i_ins;

    /* Tag settings */
    tmp = strchr(i_ins->tag, '*');
    if (tmp) {
        ctx->dynamic_tag = FLB_TRUE;
    }
    else {
        ctx->dynamic_tag = FLB_FALSE;
    }
    ctx->i_ins->flags |= FLB_INPUT_DYN_TAG;

    /* Database file */
    tmp = flb_input_get_property("db", i_ins);
    if (tmp) {
        ctx->db = flb_systemd_db_open(tmp, i_ins, config);
        if (!ctx->db) {
            flb_error("[in_systemd] could not open/create database");
        }
    }

    /* Max number of entries per notification */
    tmp = flb_input_get_property("max_entries", i_ins);
    if (tmp) {
        ctx->max_entries = atoi(tmp);
    }
    else {
        ctx->max_entries = FLB_SYSTEND_ENTRIES;
    }

    /* Load Systemd filters, iterate all properties */
    mk_list_foreach(head, &i_ins->properties) {
        prop = mk_list_entry(head, struct flb_config_prop, _head);
        if (strcasecmp(prop->key, "systemd_filter") != 0) {
            continue;
        }

        /* Apply filter/match */
        sd_journal_add_match(ctx->j, prop->val, 0);
    }

    /* Always seek to head */
    sd_journal_seek_head(ctx->j);

    /* Check if we have a cursor in our database */
    if (ctx->db) {
        tmp = flb_systemd_db_get_cursor(ctx);
        if (tmp) {
            ret = sd_journal_seek_cursor(ctx->j, tmp);
            if (ret == 0) {
                flb_info("[in_systemd] seek_cursor=%.40s... OK", tmp);
            }
            else {
                flb_warn("[in_systemd] seek_cursor failed");
            }
            flb_free(tmp);
        }
    }

    return ctx;
}

int flb_systemd_config_destroy(struct flb_systemd_config *ctx)
{
    /* Close context */
    if (ctx->j) {
        sd_journal_close(ctx->j);
    }

    if (ctx->db) {
        flb_systemd_db_close(ctx->db);
    }

    flb_free(ctx);
    return 0;
}
