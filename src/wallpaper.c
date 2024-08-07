#include <stdio.h>
#include "bing_wallpaper.h"
#include <windows.h>
 #include <curl/curl.h>
/*Regions
en-US
en-GB
cn-ZH
ja-JP
en-CA
en-AU ROW
en-NZ ROW
de-DE
es-ES
fr-FR
it-IT
pt-BR
*/

int main()
{
  getTodayPhoto("en-US");
  // curl_global_init(CURL_GLOBAL_ALL);
  // curl_global_cleanup();
  // while (1){
  //   Sleep(100000);
  // }
  
  printf("done");
  return 0;
}   


