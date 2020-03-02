/*
 * librdkafka - The Apache Kafka C/C++ library
 *
 * Copyright (c) 2020 Magnus Edenhill
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


/**
 * @name Public API complex error type implementation.
 *
 */

#include "rdkafka_int.h"
#include "rdkafka_error.h"

#include <stdarg.h>


void rd_kafka_error_destroy (rd_kafka_error_t *error) {
        rd_free(error);
}


/**
 * @brief Creates a new error object using the optional va-args format list.
 */
rd_kafka_error_t *rd_kafka_error_new_v (rd_kafka_resp_err_t code,
                                        const char *fmt, va_list ap) {
        rd_kafka_error_t *error;
        ssize_t strsz = 0;
        va_list ap2;

        if (fmt && *fmt) {
                va_copy(ap2, ap);
                strsz = rd_vsnprintf(NULL, 0, fmt, ap1) + 1;
        }

        error = rd_malloc(sizeof(*error) + strsz);
        error->code = code;
        error->fatal = rd_false;
        error->txn_abortable = rd_false;

        if (fmt && *fmt) {
                error->errstr = (char *)(error+1);
                rd_vsnprintf(error->errstr, strsz, fmt, ap2);
                va_end(ap2);
        } else {
                error->errstr = NULL;
        }

        return error;
}

rd_kafka_error_t *rd_kafka_error_new (rd_kafka_resp_err_t code,
                                      const char *fmt, ...) {
        rd_kafka_error_t *error;
        va_list ap;

        va_start(ap, fmt);
        error = rd_kafka_error_new_v(code, fmt, ap);
        va_end(ap);

        return error;
}

rd_kafka_resp_err_t rd_kafka_error_code (const rd_kafka_error_t *error) {
        return error->code;
}

const char *rd_kafka_error_string (const rd_kafka_error_t *error) {
        return error->errstr ? error->errstr : rd_kafka_err2str(error->code);
}

int rd_kafka_error_is_fatal (const rd_kafka_error_t *error) {
        return error->fatal ? 1 : 0;
}

int rd_kafka_error_is_txn_abortable (const rd_kafka_error_t *error) {
        return error->txn_abortable ? 1 : 0;
}


/**
 * @brief Converts a new style error_t error to the legacy style
 *        resp_err_t code and separate error string, then
 *        destroys the the error object.
 *
 * @remark The \p error object is destroyed.
 */
rd_kafka_resp_err_t
rd_kafka_error_to_legacy (rd_kafka_error_t *error,
                          char *errstr, size_t errstr_size) {
        rd_kafka_resp_err_t err = error->code;

        rd_snprintf(errstr, errstr_size, "%s", rd_kafka_error_string(error));

        rd_kafka_error_destroy(error);

        return err;
}

/**@}*/
