/***************************************************************************
                          menu_hardware_te1_card_advanced_options.cpp  -  description
                             -------------------
    begin                : Thu Apr 1 2004
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

#include "menu_hardware_te1_card_advanced_options.h"
#include "text_box.h"
#include "menu_te1_select_media.h"
#include "menu_te_select_line_decoding.h"
#include "menu_te_select_framing.h"
#include "menu_t1_lbo.h"
#include "input_box_active_channels.h"
#include "menu_te1_clock_mode.h"

char* active_te1_channels_help_str =
"\n"
"BOUND T1/E1 ACTIVE CHANNELS\n"
"\n"
"Please specify T1/E1 channles to be used.\n"
"\n"
"You can use the follow format:\n"
"o ALL     - for full T1/E1 links\n"
"o x.a-b.w - where the letters are numbers; to specify\n"
"            the channels x and w and the range of\n"
"            channels a-b, inclusive for fractional\n"
"            links.\n"
" Example:\n"
"o ALL     - All channels will be selcted\n"
"o 1       - A first channel will be selected\n"
"o 1.3-8.12 - Channels 1,3,4,5,6,7,8,12 will be selected.\n"
"\n";

char* te1_options_help_str =
"T1/E1 LINE DECODING\n"
"\n"
"for T1:\n"
" AMI or B8ZS\n"
"for E1:\n"
" AMI or HDB3\n"
"\n"
"T1/E1 FRAMIING MODE\n"
"\n"
"for T1:\n"
" D4 or ESF or UNFRAMED\n"
"for E1:\n"
" non-CRC4 or CRC4 or UNFRAMED\n"
"\n"
"T1/E1 CLOCK MODE\n"
"\n"
" Normal - normal clock mode (default)\n"
" Master - master clock mode\n"
"\n"
"T1 LINE BUILD OUT\n"
"\n"
"for Long Haul\n"
" 1.  CSU: 0dB\n"
" 2.  CSU: 7.5dB\n"
" 3.  CSU: 15dB\n"
" 4.  CSU: 22.5dB\n"
"for Short Haul\n"
" 5.  DSX: 0-110ft\n"
" 6.  DSX: 110-220ft\n"
" 7.  DSX: 220-330ft\n"
" 8.  DSX: 330-440ft\n"
" 9.  DSX: 440-550ft\n"
" 10. DSX: 550-660ft\n";


enum TE1_ADVANCED_OPTIONS{
	TE1_MEDIA=1,
	TE1_LCODE,
	TE1_FRAME,
	T1_LBO,
	TE1_CLOCK,
	TE1_ACTIVE_CH
};

#define DBG_MENU_HARDWARE_TE1_CARD_ADVANCED_OPTIONS 1

menu_hardware_te1_card_advanced_options::
	menu_hardware_te1_card_advanced_options(  IN char * lxdialog_path,
                                            IN conf_file_reader* ptr_cfr)
{
  Debug(DBG_MENU_HARDWARE_TE1_CARD_ADVANCED_OPTIONS,
    ("menu_hardware_te1_card_advanced_options::menu_hardware_te1_card_advanced_options()\n"));

  snprintf(this->lxdialog_path, MAX_PATH_LENGTH, "%s", lxdialog_path);
  this->cfr = ptr_cfr;
}

menu_hardware_te1_card_advanced_options::~menu_hardware_te1_card_advanced_options()
{
  Debug(DBG_MENU_HARDWARE_TE1_CARD_ADVANCED_OPTIONS,
    ("menu_hardware_te1_card_advanced_options::~menu_hardware_te1_card_advanced_options()\n"));
}

int menu_hardware_te1_card_advanced_options::run(OUT int * selection_index)
{
  string menu_str;
  int rc;
  char tmp_buff[MAX_PATH_LENGTH];
  unsigned int option_selected;
  char exit_dialog;
  int number_of_items;

  //help text box
  text_box tb;

  link_def_t * link_def;
  wandev_conf_t *linkconf;

  input_box_active_channels act_channels_ip;

  Debug(DBG_MENU_HARDWARE_TE1_CARD_ADVANCED_OPTIONS, ("menu_net_interface_setup::run()\n"));

again:
  rc = YES;
  option_selected = 0;
  exit_dialog = NO;

  link_def = cfr->link_defs;
  linkconf = cfr->link_defs->linkconf;

  Debug(DBG_MENU_HARDWARE_TE1_CARD_ADVANCED_OPTIONS,
    ("cfr->link_defs->name: %s\n", link_def->name));

  if(linkconf->fe_cfg.media == WAN_MEDIA_T1){
    number_of_items = 6;
  }else if(linkconf->fe_cfg.media == WAN_MEDIA_E1){
    number_of_items = 5;
  }else{
    ERR_DBG_OUT(("Unknown Media Type!! te_cfg.media: 0x%X\n", linkconf->fe_cfg.media));
    return NO;
  }

  menu_str = "";

  //////////////////////////////////////////////////////////////////////////////////////
  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", TE1_MEDIA);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"Physical Medium--> %s\" ",
    		MEDIA_DECODE(linkconf->fe_cfg.media));
  menu_str += tmp_buff;

  //////////////////////////////////////////////////////////////////////////////////////

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", TE1_LCODE);
  menu_str += tmp_buff;

  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"Line decoding----> %s\" ", 
		  	LCODE_DECODE(linkconf->fe_cfg.lcode));
  menu_str += tmp_buff;

  //////////////////////////////////////////////////////////////////////////////////////
  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", TE1_FRAME);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"Framing----------> %s\" ", 
		  FRAME_DECODE(linkconf->fe_cfg.frame));
  menu_str += tmp_buff;

  //////////////////////////////////////////////////////////////////////////////////////
  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", TE1_CLOCK);
  menu_str += tmp_buff;
  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"TE clock mode----> %s\" ",
    		TECLK_DECODE(linkconf->fe_cfg.cfg.te_cfg.te_clock));
#if 0
  snprintf(tmp_buff, MAX_PATH_LENGTH, " \"TE clock mode----> %s\" ",
    (linkconf->fe_cfg.cfg.te_cfg.te_clock == WANOPT_NORMAL_CLK ? "Normal" : "Master"));
#endif
  menu_str += tmp_buff;
  //////////////////////////////////////////////////////////////////////////////////////

  //on AFT it must be 'ALL' because channelization is done on per-group-of-channels basis.
  //on S514-cards it is configurable here
  if(linkconf->card_type == WANOPT_S51X){
    
    snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", TE1_ACTIVE_CH);
    menu_str += tmp_buff;
    snprintf(tmp_buff, MAX_PATH_LENGTH, " \"Act. channels----> %s\" ",
      link_def->active_channels_string);
    menu_str += tmp_buff;
  }

  //////////////////////////////////////////////////////////////////////////////////////
  if(linkconf->fe_cfg.media == WAN_MEDIA_T1){
    snprintf(tmp_buff, MAX_PATH_LENGTH, " \"%d\" ", T1_LBO);
    menu_str += tmp_buff;
    snprintf(tmp_buff, MAX_PATH_LENGTH, " \"LBO--------------> %s\" ", 
		    LBO_DECODE(linkconf->fe_cfg.cfg.te_cfg.lbo));
    menu_str += tmp_buff;
  }

  //////////////////////////////////////////////////////////////////////////////////////
  //create the explanation text for the menu
  snprintf(tmp_buff, MAX_PATH_LENGTH,
"\n------------------------------------------\
\nAdvanced Hardware settings for: %s.", link_def->name);

  if(set_configuration(   YES,//indicates to call V2 of the function
                          MENU_BOX_BACK,//MENU_BOX_SELECT,
                          lxdialog_path,
                          "T1/E1 ADVANCED CONFIGURATION OPTIONS",
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
    Debug(DBG_MENU_HARDWARE_TE1_CARD_ADVANCED_OPTIONS,
      ("hardware_setup: option selected for editing: %s\n", get_lxdialog_output_string()));

    switch(atoi(get_lxdialog_output_string()))
    {
    case TE1_MEDIA:
      {
        menu_te1_select_media te1_select_media(lxdialog_path, cfr);
        if(te1_select_media.run(selection_index) == NO){
          rc = NO;
          exit_dialog = YES;
        }
      }
      break;

    case TE1_LCODE:
      {
        menu_te_select_line_decoding te1_select_line_decoding(lxdialog_path, cfr);
        if(te1_select_line_decoding.run(selection_index) == NO){
          rc = NO;
          exit_dialog = YES;
        }
      }
      break;

    case TE1_FRAME:
      {
        menu_te_select_framing te1_select_framing(lxdialog_path, cfr);
        if(te1_select_framing.run(selection_index) == NO){
          rc = NO;
          exit_dialog = YES;
        }
      }
      break;

    case T1_LBO:
      {
        menu_t1_lbo t1_lbo(lxdialog_path, cfr);
        if(t1_lbo.run(selection_index) == NO){
          rc = NO;
          exit_dialog = YES;
        }
      }
      break;

    case TE1_CLOCK:
      {
        menu_te1_clock_mode te1_clock_mode(lxdialog_path, cfr);
        if(te1_clock_mode.run(selection_index) == NO){
          rc = NO;
          exit_dialog = YES;
        }
      }
      break;

    case TE1_ACTIVE_CH:
      Debug(DBG_MENU_HARDWARE_TE1_CARD_ADVANCED_OPTIONS,
        ("cfr->link_defs->linkconf->config_id: %d\n", cfr->link_defs->linkconf->config_id));
      if(cfr->link_defs->linkconf->config_id == WANCONFIG_AFT){
        //must be alwayse ALL channels
        break;
      }

      if(act_channels_ip.show(  lxdialog_path,
                                linkconf->config_id,
                                link_def->active_channels_string,//initial text
                                selection_index,
                                linkconf->fe_cfg.media) == NO){
        rc = NO;
      }else{

        switch(*selection_index)
        {
        case INPUT_BOX_BUTTON_OK:
          snprintf(link_def->active_channels_string, MAX_LEN_OF_ACTIVE_CHANNELS_STRING,
                act_channels_ip.get_lxdialog_output_string());

          break;

        case INPUT_BOX_BUTTON_HELP:
          tb.show_help_message(lxdialog_path, WANCONFIG_AFT, active_te1_channels_help_str);
          break;
        }
        goto again;
      }
      break;

    default:
      ERR_DBG_OUT(("Invalid option selected for editing!! selection: %s\n",
        get_lxdialog_output_string()));
      rc = NO;
      exit_dialog = YES;
    }
    break;

  case MENU_BOX_BUTTON_HELP:
    tb.show_help_message(lxdialog_path, NO_PROTOCOL_NEEDED, te1_options_help_str);
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
