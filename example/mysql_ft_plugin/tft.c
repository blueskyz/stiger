/* Copyright (c) 2005, 2011, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <stdlib.h>
#include <ctype.h>
#include <mysql/plugin.h>

#include <st_darts.h>
#include <st_utils.h>

#if !defined(__attribute__) && (defined(__cplusplus) || !defined(__GNUC__)  || __GNUC__ == 2 && __GNUC_MINOR__ < 8)
#define __attribute__(A)
#endif

static long number_of_calls= 0; /* for SHOW STATUS, see below */

/*
  Simple full-text parser plugin that acts as a replacement for the
  built-in full-text parser:
  - All non-whitespace characters are significant and are interpreted as
   "word characters."
  - Whitespace characters are space, tab, CR, LF.
  - There is no minimum word length.  Non-whitespace sequences of one
    character or longer are words.
  - Stopwords are used in non-boolean mode, not used in boolean mode.
*/

/*
  tft interface functions:

  Plugin declaration functions:
  - tft_plugin_init()
  - tft_plugin_deinit()

  Parser descriptor functions:
  - tft_parse()
  - tft_init()
  - tft_deinit()
*/


static char* g_s_dictFile="/opt/mysql/dict_chs.dic";
static st_darts* g_s_pDarts = NULL;

/*
  Initialize the parser plugin at server start or plugin installation.

  SYNOPSIS
    tft_plugin_init()

  DESCRIPTION
    Does nothing.

  RETURN VALUE
    0                    success
    1                    failure (cannot happen)
*/

static int tft_plugin_init(void *arg __attribute__((unused)))
{
  g_s_pDarts = stDartsLoad(g_s_dictFile);
  stLog("load tft plugin succ.");
  return(0);
}


/*
  Terminate the parser plugin at server shutdown or plugin deinstallation.

  SYNOPSIS
    tft_plugin_deinit()
    Does nothing.

  RETURN VALUE
    0                    success
    1                    failure (cannot happen)

*/

static int tft_plugin_deinit(void *arg __attribute__((unused)))
{
  stDartsFree(g_s_pDarts);
  stLog("free tft plugin succ.");
  return(0);
}


/*
  Initialize the parser on the first use in the query

  SYNOPSIS
    tft_init()

  DESCRIPTION
    Does nothing.

  RETURN VALUE
    0                    success
    1                    failure (cannot happen)
*/

static int tft_init(MYSQL_FTPARSER_PARAM *param __attribute__((unused)))
{
  return(0);
}


/*
  Terminate the parser at the end of the query

  SYNOPSIS
    tft_deinit()

  DESCRIPTION
    Does nothing.

  RETURN VALUE
    0                    success
    1                    failure (cannot happen)
*/

static int tft_deinit(MYSQL_FTPARSER_PARAM *param __attribute__((unused)))
{
  return(0);
}


/*
  Pass a word back to the server.

  SYNOPSIS
    add_word()
      param              parsing context of the plugin
      word               a word
      len                word length

  DESCRIPTION
    Fill in boolean metadata for the word (if parsing in boolean mode)
    and pass the word to the server.  The server adds the word to
    a full-text index when parsing for indexing, or adds the word to
    the list of search terms when parsing a search string.
*/

static void add_word(MYSQL_FTPARSER_PARAM *param, char *word, size_t len)
{
  MYSQL_FTPARSER_BOOLEAN_INFO bool_info=
  { FT_TOKEN_WORD, 0, 0, 0, 0, ' ', 0 };
  if (param->mode == MYSQL_FTPARSER_FULL_BOOLEAN_INFO){
    bool_info.yesno = 1;
  }
  param->mysql_add_word(param, word, len, &bool_info);
}

/*
  Parse a document or a search query.

  SYNOPSIS
    tft_parse()
      param              parsing context

  DESCRIPTION
    This is the main plugin function which is called to parse
    a document or a search query. The call mode is set in
    param->mode.  This function simply splits the text into words
    and passes every word to the MySQL full-text indexing engine.
*/

static int tft_parse_en(MYSQL_FTPARSER_PARAM *param)
{
  char *end, *start, *docend= param->doc + param->length;

  number_of_calls++;

  for (end= start= param->doc;; end++)
  {
    if (end == docend)
    {
      if (end > start)
        add_word(param, start, end - start);
      break;
    }
    else if (isspace(*end))
    {
      if (end > start)
        add_word(param, start, end - start);
      start= end + 1;
    }
  }
  return 0;
}

#define c_uWordsCount 1024
static int tft_parse(MYSQL_FTPARSER_PARAM *param)
{
  if (NULL == param->doc || 0 == param->length){
    return 0;
  }

  number_of_calls++;

  st_timer stTimerType = ST_TIMER_MICRO_SEC;
  char* start = param->doc;
  char* end = param->doc;
  char* docend = param->doc + param->length;
  struct st_wordInfo wordInfo[c_uWordsCount] = { { 0, 0, 0 } };
  // param->flags = MYSQL_FTFLAGS_NEED_COPY;

  st_darts_state dState;
  char output[1024] = {0};
  memcpy(output, param->doc, min(param->length, 1024));
  output[1024] = '\0';
  
  // stConvertCode(encode, "utf-8", input, strlen(input), output, MAX_PATH);
  stDartsStateInit(g_s_pDarts, &dState, start, docend);
  long long queryBeginTime = stTimer(stTimerType);
  uint32_t uWordsCount = 0;
  while (uWordsCount < c_uWordsCount 
		  && stDartsNextWord(g_s_pDarts, &dState, &wordInfo[uWordsCount])){
	  ++uWordsCount;
  }
  long long queryEndTime = stTimer(stTimerType);
  stLog("input=%s, paramlen=%d, result=%u, cost time=%lldus", 
		  output,
		  param->length,
		  uWordsCount, 
		  queryEndTime - queryBeginTime);
  if(uWordsCount == 0){
    tft_parse_en(param);
  }

  char outWordId[32] = { 0 };
  for (int i = uWordsCount - 1; i >= 0; --i){
    add_word(param, wordInfo[i].pWord, wordInfo[i].wordLen);
  }

  return(0);
}


/*
  Plugin type-specific descriptor
*/

static struct st_mysql_ftparser tft_descriptor=
{
  MYSQL_FTPARSER_INTERFACE_VERSION, /* interface version      */
  tft_parse,              /* parsing function       */
  tft_init,               /* parser init function   */
  tft_deinit              /* parser deinit function */
};

/*
  Plugin status variables for SHOW STATUS
*/

static struct st_mysql_show_var tft_status[]=
{
  {"static",     (char *)"just a static text",     SHOW_CHAR},
  {"called",     (char *)&number_of_calls, SHOW_LONG},
  {0,0,0}
};

/*
  Plugin system variables.
*/

static long     sysvar_one_value;
static char     *sysvar_two_value;

static MYSQL_SYSVAR_LONG(tft_sysvar_one, sysvar_one_value,
  PLUGIN_VAR_RQCMDARG,
  "tft fulltext parser system variable number one. Give a number.",
  NULL, NULL, 77L, 7L, 777L, 0);

static MYSQL_SYSVAR_STR(tft_sysvar_two, sysvar_two_value,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
  "tft fulltext parser system variable number two. Give a string.",
  NULL, NULL, "tft sysvar two default");

static MYSQL_THDVAR_LONG(tft_thdvar_one,
  PLUGIN_VAR_RQCMDARG,
  "tft fulltext parser thread variable number one. Give a number.",
  NULL, NULL, 88L, 8L, 888L, 0);

static MYSQL_THDVAR_STR(tft_thdvar_two,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
  "tft fulltext parser thread variable number two. Give a string.",
  NULL, NULL, "tft thdvar two default");

static struct st_mysql_sys_var* tft_system_variables[]= {
  MYSQL_SYSVAR(tft_sysvar_one),
  MYSQL_SYSVAR(tft_sysvar_two),
  MYSQL_SYSVAR(tft_thdvar_one),
  MYSQL_SYSVAR(tft_thdvar_two),
  NULL
};

/*
  Plugin library descriptor
*/

mysql_declare_plugin(tft)
{
  MYSQL_FTPARSER_PLUGIN,      /* type                            */
  &tft_descriptor,  /* descriptor                      */
  "tft",            /* name                            */
  "t Corp",              /* author                          */
  "t Full-Text Parser",  /* description                     */
  PLUGIN_LICENSE_GPL,
  tft_plugin_init,  /* init function (when loaded)     */
  tft_plugin_deinit,/* deinit function (when unloaded) */
  0x0100,                     /* version                         */
  tft_status,              /* status variables                */
  tft_system_variables,    /* system variables                */
  NULL,
  0,
}
mysql_declare_plugin_end;

