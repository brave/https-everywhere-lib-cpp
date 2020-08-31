#ifndef HTTPS_EVERYWHERE_LIB_CPP_H
#define HTTPS_EVERYWHERE_LIB_CPP_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * A C-compatible enumerated type representing possible actions taken as the result of a rewrite
 * operation.
 */
typedef enum {
  NoOp = 0,
  RewriteUrl = 1,
} C_RewriteActionEnum;

/**
 * Main struct accessible as a pointer across the FFI.
 *
 * `rewriter` is `None` until rules have been added for initialization.
 */
typedef struct C_HttpseClient C_HttpseClient;

/**
 * A C-compatible return type representing the result of a rewrite operation.
 */
typedef struct {
  C_RewriteActionEnum action;
  const char *new_url;
} C_RewriteResult;

/**
 * Initializes the `HttpseClient` with a set of rules, in JSON format.
 *
 * The rules are in the same format as the `rulesets` key of the EFF's official lists.
 *
 * Returns whether or not the operation was successful.
 *
 * # Safety
 * This function will cause undefined behavior if `client` or `rules` do not point to properly
 * initialized memory.
 */
bool initialize_client(C_HttpseClient *client, const char *rules);

/**
 * Creates a new HttpseClient for use across the FFI.
 */
C_HttpseClient *new_client(void);

/**
 * Use the `HttpseClient` to rewrite the given URL according to any applicable rules.
 *
 * # Safety
 * This function will cause undefined behavior if `client` or `url` do not point to properly
 * initialized memory.
 */
C_RewriteResult rewriter_rewrite_url(C_HttpseClient *client, const char *url);

#endif /* HTTPS_EVERYWHERE_LIB_CPP_H */
