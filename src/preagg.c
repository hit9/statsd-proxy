#include "preagg.h"

void load_preagg_rules(char *file, struct agg_rules *rules) {
  rules->rules = malloc(sizeof(struct agg_rule) * MAX_AGG_RULE_NUM);
  rules->rules_num = 0;
  FILE *fp;
  if ((fp = fopen(file, "r")) == NULL) {
    log_warn("pre aggregator config file not found");
    return;
  }

  size_t now_pos = 0;
  char prefix[MAX_KEY_LENGTH] = {};
  char to[MAX_KEY_LENGTH] = {};
  char rule_type[MAX_RULE_TYPE_LENGTH] = {};
  memset(prefix, '\0', MAX_KEY_LENGTH);
  memset(to, '\0', MAX_KEY_LENGTH);
  memset(rule_type, '\0', MAX_RULE_TYPE_LENGTH);
  log_debug("read config file %s", file);

  while (fscanf(fp, "%s %s %s", rule_type, prefix, to) != EOF) {
    size_t len_prefix = strlen(prefix);
    size_t len_to = strlen(prefix);
    size_t len_type = strlen(rule_type);
    char *prefix_cpy = malloc(len_prefix);
    char *to_cpy = malloc(len_to);
    memmove(prefix_cpy, prefix, len_prefix);
    memmove(to_cpy, to, len_to);
    struct agg_rule *rule = malloc(sizeof(struct agg_rule));
    rule->prefix = prefix_cpy;
    rule->prefix_len = len_prefix;
    rule->to = to_cpy;
    rule->to_len = len_to;
    if (len_type != 0) {
      switch (rule_type[0]) {
        case 'r':
          rule->rule_type = AGG_REMOVE;
          break;
        case 'c':
        default:
          rule->rule_type = AGG_COPY;
      }
    } else {
      log_warn("Invalidate Rule Type");
      continue;
    }

    rules->rules[now_pos++] = rule;

    memset(rule_type, '\0', len_type);
    memset(prefix, '\0', len_prefix);
    memset(to, '\0', len_to);
  }
  rules->rules_num = now_pos;

  free(prefix);
  free(to);
  fclose(fp);
}

size_t is_prefix(const char *str1, size_t len_str1, const char *str2,
                 size_t len_str2);

int preagg(struct parser_result *result, struct parser_result *out_result) {
  size_t i = 0;
  for (; i < PREAGG_RULES.rules_num; i++) {
    struct agg_rule *rule = PREAGG_RULES.rules[i];
    if (result->len < rule->prefix_len) continue;
    // to find out if  there have the special prefix
    size_t pos =
        is_prefix(rule->prefix, rule->prefix_len, result->key, result->len);
    if (!pos) continue;

    size_t left_blen = result->blen - pos;
    size_t alloc_blen = rule->to_len + left_blen;
    char *blk = malloc(alloc_blen);
    memcpy(blk, rule->to, rule->to_len);
    memcpy(&blk[rule->to_len], &result->block[pos], left_blen);

    size_t left_len = result->len - 1 - pos;
    size_t key_len = rule->to_len + left_len;

    out_result->block = blk;
    out_result->blen = alloc_blen;
    out_result->key = blk;
    out_result->len = key_len;
    return rule->rule_type;
  }

  return AGG_NOAGG;
}

/*
 * Return the pos where should we substr if  str1 is the prefix of str2
 * Return 0 if  len_str2 < len_str1 or str1 is not the prefix of str2
 */
size_t is_prefix(const char *str1, size_t len_str1, const char *str2,
                 size_t len_str2) {
  if (len_str2 < len_str1) return 0;
  size_t pos = 0;
  size_t matched = 0;
  for (; pos < len_str1; ++pos) {
    if (str1[pos] != str2[pos]) return 0;
    ++matched;
  }

  size_t next_dot_pos = matched;
  // find next dot and exclude it
  pos = matched;
  while (pos < len_str2) {
    if (str2[pos] == '.') {
      next_dot_pos = pos;
      break;
    }
    ++pos;
  }

  if (next_dot_pos == matched) {
    return len_str2 - 1;
  }
  return next_dot_pos;
}
