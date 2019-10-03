#include "preagg.h"

enum {
  RULE_STATE_START  = 0,
  RULE_STATE_TYPE   = 1,
  RULE_STATE_PREFIX = 2,
  RULE_STATE_SUFFIX = 3,
  RULE_STATE_SPLIT  = 4,
};


int parse_preagg_rules(struct agg_rules *rules, const char* value, size_t value_len) {
  int state = RULE_STATE_START;
  size_t pos = 0, mark = 0;
  struct agg_rule rule;
  while(pos < value_len) {
    switch(state){
    case RULE_STATE_START:
      state = RULE_STATE_TYPE;
      break;
    case RULE_STATE_SPLIT:
      state = RULE_STATE_TYPE;
      if (state == RULE_STATE_TYPE) {
        state = RULE_STATE_PREFIX;
        mark = pos+1;
        if (strncmp("remove", value, AGG_REMOVE_LEN) ) {
          rule.rule_type = AGG_REMOVE;
        } else if (strncmp("copy", value, AGG_COPY_LEN)) {
          rule.rule_type = AGG_COPY;
        } else {
          return AGG_ETYPE;
        }
      } else if (state == RULE_STATE_PREFIX) {
        state = RULE_STATE_SUFFIX;
        mark = pos + 1;
        rule.prefix_len = pos-mark;
        rule.prefix = malloc(rule.prefix_len);
        memcpy(&rule.prefix, &value[mark], rule.prefix_len);
      } else {
        return AGG_ESPLIT;
      }
      break;
    }
  }

  if (pos > mark) {
    rule.to_len = pos - mark;
    rule.to = malloc(rule.to_len);
    memcpy(rule.to, &value[mark], rule.to_len);
  } else if (pos == mark) {
    rule.to_len = rule.prefix_len;
    rule.to = malloc(rule.prefix_len);
    memcpy(rule.to, rule.prefix, rule.prefix_len);
  } else {
    // Unreachable, but deal it for excaption;
    return AGG_ECONF;
  }

  if (rules->rules_num > MAX_AGG_RULE_NUM) {
    return AGG_ETOOMUCH;
  }

  memcpy(&rules->rules[rules->rules_num], &rule, sizeof(struct agg_rule));
  return AGG_OK;
}

size_t is_prefix(const char *str1, size_t len_str1, const char *str2,
                 size_t len_str2);

int preagg(struct parser_result *result, struct parser_result *out_result) {
  size_t i = 0;
  for (; i < PREAGG_RULES.rules_num; i++) {
    struct agg_rule rule = PREAGG_RULES.rules[i];
    if (result->len < rule.prefix_len) continue;
    // to find out if  there have the special prefix
    size_t pos =
        is_prefix(rule.prefix, rule.prefix_len, result->key, result->len);
    if (!pos) continue;

    size_t left_blen = result->blen - pos;
    size_t alloc_blen = rule.to_len + left_blen;
    char *blk = malloc(alloc_blen);
    memcpy(blk, rule.to, rule.to_len);
    memcpy(&blk[rule.to_len], &result->block[pos], left_blen);

    size_t left_len = result->len - 1 - pos;
    size_t key_len = rule.to_len + left_len;

    out_result->block = blk;
    out_result->blen = alloc_blen;
    out_result->key = blk;
    out_result->len = key_len;
    return rule.rule_type;
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
