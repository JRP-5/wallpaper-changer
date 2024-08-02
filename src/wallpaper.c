#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
 
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}
 
int main()
{
  printf("Hello world");
  CURL *curl_handle;
  static const char *pagefilename = "images/pic.jpg";
  FILE *pagefile;

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* set URL to get here */
  curl_easy_setopt(curl_handle, CURLOPT_URL, "https://bing.gifposter.com/bingImages/HoodoosBryce_uhd.jpg");

  /* Switch on full protocol/debug output while testing */
  curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

  /* disable progress meter, set to 0L to enable it */
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "lib/curl-ca-bundle.crt");

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

  /* open the file */
  pagefile = fopen(pagefilename, "wb");
  if(pagefile) {

      /* write the page body to this file handle */
      curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

      /* get it! */
      curl_easy_perform(curl_handle);

      /* close the header file */
      fclose(pagefile);
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  curl_global_cleanup();
  printf("Hello world");
  return 0;
}   


