/***************************************************************************
                          menu_t1_lbo.cpp  -  description
                             -------------------
    begin                : Wed Apr 28 2004
    copyright            : (C) 2004 by David Rokhvarg
    email                : davidr@sangoma.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "menu_t1_lbo.h"
#include "text_box.h"

#define DBG_MENU_T1_LBO 1

menu_t1_lbo::menu_t1_lbo(IN char * lxdialog_path, IN conf_file_reader* ptr_cfr)
{
  Debug(DBG_MENU_T1_LBO, ("menu_t1_lbo::menu_t1_lbo()\n"));

  snprintf(this->lxdialog_path, MAX_PATH_LENGTH, "%s", lxdialog_path);
  this->cfr = ptr_cfr;
}

menu_t1_lbo::~menu_t1_lbo()
{
  Debug(DBG_MENU_T1_LBO, ("menu_t1_lbo::~menu_t1_lbo()\n"));
}

int menu_t1_lbo::run(OUT int * selection_index)
{
  string menu_str;
  int rc;
  char tmp_buff[MAX_PATH_LENGTH];
  char exit_dialog;
  int number_of_items;
  unsigned char old_media;

  //help text box
  text_box tb;

  link_def_t * link_def;
  wandev_conf_t *linkconf;

  Debug(DBG_MENU_T1_LBO, ("menu_t1_lbo::run()\n"));

again:

  link_def = cfr->link_defs;
  linkconf = cfr->link_defs->linkconf;

  Debug(DBG_MENU_T1_LBO, ("cfr->link_defs->name: %s\n", link_def->name));

  rc = YES;
  exit_dialog = NO;
  old_media = linkconf->fe_cfg.media;

  menu_str = " ";

  Debug(DBG_MENU_T1_LBO, ("linkconf->card_type: DEC:%d, HEX: 0x%X\n",
    linkconf->card_type, linkconf->card_type));

  number_of_items = 10;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", WAN_T1_LBO_0_DB);
  menu_str += (char*)tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"0db\" ");
  menu_str += tmp_buff;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", WAN_T1_LBO_75_DB);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH,  "7.5dB");
  menu_str += tmp_buff;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", WAN_T1_LBO_15_DB);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH,  "15dB");
  menu_str += tmp_buff;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", WAN_T1_LBO_225_DB);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH,  "22.5dB");
  menu_str += tmp_buff;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", WAN_T1_0_110);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH,  "0-110ft");
  menu_str += tmp_buff;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", WAN_T1_110_220);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH,  "110-220ft");
  menu_str += tmp_buff;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", WAN_T1_220_330);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH,  "220-330ft");
  menu_str += tmp_buff;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", WAN_T1_330_440);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH,  "330-440ft");
  menu_str += tmp_buff;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", WAN_T1_440_550);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH,  "440-550ft");
  menu_str += tmp_buff;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", WAN_T1_550_660);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH,  "550-660ft");
  menu_str += tmp_buff;

  //////////////////////////////////////////////////////////////////////////////////////
  //create the explanation text for the menu
  snprintf(tmp_buff, MAX_PATH_LENGTH,
"\n------------------------------------------\
\nSelect Line Build Out for Wan Device: %s", link_def->name);

  if(set_configuration(   YES,//indicates to call V2 of the function
                          MENU_BOX_BACK,//MENU_BOX_SELECT,
                          lxdialog_path,
                          "SELECT LINE BUILD OUT",
                          WANCFG_PROGRAM_NAME,
                          tmp_buff,
                          MENU_HEIGTH, MENU_WIDTH,
                          number_of_items,
                          (char*)menu_str.c_str()
                          ) == NO){
    rc = NO;
    goto cleanup;
  }

  if(show(selection_index) == NO){
    rc = NO;
    goto cleanup;
  }
  //////////////////////////////////////////////////////////////////////////////////////

  exit_dialog = NO;
  switch(*selection_index)
  {
  case MENU_BOX_BUTTON_SELECT:
    Debug(DBG_MENU_T1_LBO,
      ("hardware_setup: option selected for editing: %s\n", get_lxdialog_output_string()));

    linkconf->fe_cfg.cfg.te_cfg.lbo = atoi(get_lxdialog_output_string());
    exit_dialog = YES;
    break;

  case MENU_BOX_BUTTON_HELP:

    switch(atoi(get_lxdialog_output_string()))
    {
    default:
      tb.show_help_message(lxdialog_path, NO_PROTOCOL_NEEDED,
        option_not_implemented_yet_help_str);
      break;
    }
    break;

  case MENU_BOX_BUTTON_EXIT:
    exit_dialog = YES;
    break;
  }//switch(*selection_index)


  if(exit_dialog == NO){
    goto again;
  }

cleanup:
  return rc;
}
