#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

struct MemoryStruct
{
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr)
    {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}
// Function which given the bing json returns the date of the first photo
// Returns null pointer if no date is found
char *getDateFromJson(char *json, size_t json_length)
{
    // Search for the startdate attribute
    int dateIndex = -1;
    for (int i = 0; i < json_length - 4; i++)
    {
        if (json[i] == 's' && json[i + 1] == 't' && json[i + 2] == 'a' && json[i + 3] == 'r' && json[i + 4] == 't')
        {
            dateIndex = i + 12;
        }
    }
    if (dateIndex != -1)
    {
        // Date is 8 letters then terminating character
        char *date = malloc(9 * sizeof(char));
        for (int i = 0; i < 8; i++)
        {
            date[i] = json[dateIndex + i];
        }
        date[8] = (char)0;
        return date;
    }
    return NULL;
}

char *replaceResolution(char *string)
{
    // Because we are removing the 1920x1080 we know the new string will not be larger than the old
    char *newString = malloc(strlen(string) * sizeof(char));
    // search for the 1920
    int index = -1;

    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] == '1' && string[i + 1] == '9' && string[i + 2] == '2' && string[i + 3] == '0')
        {
            index = i;
            break;
        }
    }
    if (index != -1)
    {
        int count = 0;
        while (count < index)
        {
            newString[count] = string[count];
            count++;
        }
        // Add in the uhd
        newString[count] = 'U';
        count++;
        newString[count] = 'H';
        count++;
        newString[count] = 'D';
        count++;
        // Add in the rest after the 1920x1080
        while (count < strlen(string))
        {
            newString[count] = string[count + 6];
            count++;
        }
        char *result = malloc(count * sizeof(char));
        for (int i = 0; i < count; i++)
        {
            result[i] = newString[i];
        }
        return result;
    }
    return string;
}
char *getURLFromJson(char *json, size_t json_length)
{
    bool foundURL = false;
    bool inURL = false;
    char *url = malloc(json_length * sizeof(char));
    int urlSize = 0;
    // Find the URL attribute in the json
    for (int i = 0; i < json_length; i++)
    {
        // If not found the url keep searching
        if (foundURL == false)
        {
            if (json[i] == 'u' && json[i + 1] == 'r' && json[i + 2] == 'l')
            {
                foundURL = true;
            }
        }
        // If already found the url add to the url
        else
        {
            if (inURL == false && json[i] == ':')
            {
                inURL = true;
                i++;
                i++;
            }
            if (inURL)
            {
                if (json[i] == '&')
                {
                    url[urlSize] = (char)0;
                    urlSize += 1;
                    inURL = false;
                    break;
                }
                else
                {
                    url[urlSize] = json[i];
                    urlSize += 1;
                }
            }
        }
    }
    // If we have found a url
    char *finalURL = malloc((urlSize + 1) * sizeof(char));
    for (int i = 0; i < urlSize; i++)
    {
        finalURL[i] = url[i];
    }
    free(url);
    return finalURL;
}

FILE getTodayPhoto(char *region)
{
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl_handle = curl_easy_init();
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1); /* grown as needed by the realloc above */
    chunk.size = 0;
    char *jsonURL = "https://www.bing.com/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=";
    //Add in the region to our request
    char *jsonURLRegion = malloc((strlen(jsonURL) + strlen(region) + 1) * sizeof(char));
    strcpy(jsonURLRegion, "");
    strcat(jsonURLRegion, jsonURL);
    strcat(jsonURLRegion, region);
    jsonURLRegion[strlen(jsonURL) + strlen(region)] = (char)0;
    curl_easy_setopt(curl_handle, CURLOPT_URL, jsonURLRegion);

    /* Switch on full protocol/debug output while testing */
    // curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

    /* disable progress meter, set to 0L to enable it */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "lib/curl-ca-bundle.crt");

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    res = curl_easy_perform(curl_handle);

    char *url = getURLFromJson(chunk.memory, chunk.size);

    // Get the UHD version of the URL
    char *uhdUrl = replaceResolution(url);
    // Add the bing bit to the start
    char *bingPrefix = "https://www.bing.com";
    char *fullUHDurl = malloc((strlen(bingPrefix) + strlen(uhdUrl) + 1) * sizeof(char));
    for (int i = 0; i < strlen(bingPrefix) + strlen(uhdUrl); i++)
    {
        if (i < strlen(bingPrefix))
        {
            fullUHDurl[i] = bingPrefix[i];
        }
        else
        {
            fullUHDurl[i] = uhdUrl[i - strlen(bingPrefix)];
        }
    }
    fullUHDurl[strlen(bingPrefix) + strlen(uhdUrl)] = (char)0;
    free(uhdUrl);

    // Fetch the photo from the url
    /* set URL to get here */
    curl_easy_setopt(curl_handle, CURLOPT_URL, fullUHDurl);
    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    char *imgDate = getDateFromJson(chunk.memory, chunk.size);
    char *imagesBing = "images/bing/";

    // File name is dateRegion.jpg
    char *pagefilename = malloc((strlen(imagesBing) + 8 + 5 + 4 + 1) * sizeof(char));
    strcpy(pagefilename, "");
    strcat(pagefilename, imagesBing);
    strcat(pagefilename, imgDate);
    strcat(pagefilename, region);
    strcat(pagefilename, ".jpg");
    pagefilename[strlen(imagesBing) + 17] = (char)0;
    // If bing directory doesn't exist create it
    struct stat st;
    if(stat("images/bing",&st) == -1) {
           mkdir("images/bing");
    }
    /* open the file */
    FILE *pagefile = fopen(pagefilename, "wb");
    if (pagefile)
    {

        /* write the page body to this file handle */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

        /* get it! */
        curl_easy_perform(curl_handle);

        /* close the header file */
        fclose(pagefile);
    }
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
}