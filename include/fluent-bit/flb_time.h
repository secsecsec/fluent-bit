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

#ifndef FLB_TIME_H
#define FLB_TIME_H

#include <time.h>
struct flb_time {
    struct timespec tm;
};

#undef FLB_TIME_FORCE_FMT_INT
#ifdef TARGET_OS_MAC
#define FLB_TIME_FORCE_FMT_INT /* FIXME */
#endif

/* 
   to represent eventtime of fluentd
   see also
   https://github.com/fluent/fluentd/wiki/Forward-Protocol-Specification-v0#eventtime-ext-format
   https://github.com/fluent/fluentd/wiki/Forward-Protocol-Specification-v1#eventtime-ext-format
 */
enum flb_time_eventtime_fmt {
    FLB_TIME_ETFMT_INT = 1,   /* second(integer) only */
    FLB_TIME_ETFMT_V0,        /* EventTime (v0) */
    FLB_TIME_ETFMT_V1_EXT,    /* EventTime (v1 ext) */
    FLB_TIME_ETFMT_V1_FIXEXT, /* EventTime (v1 fixext) */
    FLB_TIME_ETFMT_OTHER,
};

double flb_time_to_double(struct flb_time *tm);
int flb_time_diff(struct flb_time *time1,
                  struct flb_time *time0,struct flb_time *result);
int flb_time_append_to_msgpack(struct flb_time *time, msgpack_packer *pk, int fmt);
int flb_time_pop_from_msgpack(struct flb_time *time, msgpack_unpacked *upk,
                              msgpack_object **map);

#endif /* FLB_TIME_H */