/****************************************************************************
 * xmtp2km_kiface.h :
 * Low level kernel xmtp2km interface file
 * Copyright (C) 2009  Xygnada Technology, Inc.
****************************************************************************/

#ifndef _XMTP2KM_IFACE_H
#define _XMTP2KM_IFACE_H

void xmtp2km_bs_handler (int fi, int len, unsigned char * p_rxbs, unsigned char * p_txbs);
int xmtp2km_register    (void *, char *, int (*callback)(void*, unsigned char*, int));
int xmtp2km_unregister  (int);
int xmtp2km_facility_state_change (int fi, int state);

#if 0
#define AFT_XMTP2_TAP
int xmtp2km_tap_disable(int card_iface_id);
int xmtp2km_tap_enable(int card_iface_id, void *dev, int (*frame)(void *ptr, int slot, int dir, unsigned char *data, int len));
#endif

#endif
