


#if !defined(__WANEC_API_H__)
# define __WANEC_API_H__

extern int wanec_api_init(void);
extern int wanec_api_param(char *key, char *value);
extern int wanec_api_config(char*,int);
extern int wanec_api_release(char*,int);
extern int wanec_api_enable(char*,unsigned long,int);
extern int wanec_api_disable(char*,unsigned long,int);
extern int wanec_api_bypass(char*,int,unsigned long,int);
extern int wanec_api_mode(char*,int,unsigned long,int);
extern int wanec_api_dtmf(char*,int,unsigned long,unsigned char,unsigned char,int);
extern int wanec_api_stats(char*,int,int,int,int);
extern int wanec_api_tone_load(char*,char*,int);
extern int wanec_api_tone_unload(char*,unsigned int,int);
extern int wanec_api_playout(char*,int,int,unsigned int,int);
extern int wanec_api_monitor(char*,int,int);

#endif /* __WANEC_API_H__ */
