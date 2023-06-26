#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

typedef char* string;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
void DeleteWebHook(string);
void SendMessageToWebHook(string);
void SendMessageToWithJSONWebHook(string);
void CheckAndDisplayWebhookInfo(string);
void MM(string);
size_t GetFileSize(FILE*);

int main(void)
{
    string URL = (string)malloc(sizeof(char)*1024);
    char* webhookchk = "https://discord.com/api/webhooks";
    size_t strwhlen = strlen(webhookchk);

    FILE* load = fopen("load", "r");
    if(!load)
    {
        printf("[?] file \"load\" doesn't exists or can't be opened!\n");
        printf("Enter webhook URL: ");scanf("%s", URL);
        CheckAndDisplayWebhookInfo(URL);
        MM(URL);
    }
    else
    {
        size_t siz = GetFileSize(load);
        fread(URL, siz, 1, load);
        if(strncmp(URL, webhookchk, strwhlen))
        {
            printf("[?] loading URL from file failed!\n");
            printf("Enter webhook URL: ");scanf("%s", URL);
            CheckAndDisplayWebhookInfo(URL);
            MM(URL);
        }
        else
        {
            CheckAndDisplayWebhookInfo(URL);
            MM(URL);
        }
    }
    return 0;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    cJSON *cJson, *id, *channel_id, *guild_id, *name, *token, *avatar, *user, *user_name, *user_displayname, *user_id;
    char av[1024];
    cJson = cJSON_Parse(ptr);

    id               =      cJSON_GetObjectItem(cJson, "id");
    channel_id       =      cJSON_GetObjectItem(cJson, "channel_id");
    guild_id         =      cJSON_GetObjectItem(cJson, "guild_id");
    name             =      cJSON_GetObjectItem(cJson, "name");
    token            =      cJSON_GetObjectItem(cJson, "token");
    avatar           =      cJSON_GetObjectItem(cJson, "avatar");
    user             =      cJSON_GetObjectItem(cJson, "user");
    user_name        =      cJSON_GetObjectItem(user, "username");
    user_displayname =      cJSON_GetObjectItem(user, "display_name");
    user_id          =      cJSON_GetObjectItem(user, "id");

    sprintf(av, "https://cdn.discordapp.com/avatars/%s/%s", id->valuestring, avatar->valuestring);

    printf(
            "name: %s\n"\
            "id: %s\n"\
            "guild_id: %s\n"\
            "token: %s\n"\
            "channel id %s\n"\
            "avatar url: %s\n"\
            "user name: %s\n"\
            "user displayname: %s\n"\
            "user id: %s",
            name->valuestring, id->valuestring, guild_id->valuestring, token->valuestring, channel_id->valuestring, av, user_name->valuestring, user_displayname->valuestring, user_id->valuestring
            );
    return nmemb;
}
void CheckAndDisplayWebhookInfo(string URL)
{
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = {0};
    CURLcode res;

    headers = curl_slist_append(headers, "content-type: application/json"); //header
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    curl_easy_perform(curl);
}
void MM(string URL)
{
    putc('\n', stdout);
    int c;
    printf("1. Send Message\n2. Send message by loading JSON file\n3. Delete WebHook[Danger]\n4. Exit\n"); scanf("%d", &c);
    switch (c) {
        case 1:
            SendMessageToWebHook(URL);
            break;
        case 2:
            SendMessageToWithJSONWebHook(URL);
            break;
        case 3:
            DeleteWebHook(URL);
            break;
        default:
            break;
    }
}
void DeleteWebHook(string URL)
{
    //init curl
    CURL *curl = curl_easy_init();
    CURLcode res;

    //set up the request
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_URL, URL);

    //execute
    res = curl_easy_perform(curl);

    //error check
    if(res != CURLE_OK)
        printf("[!] fail: %s", curl_easy_strerror(res));

}
void SendMessageToWebHook(string URL)
{
    //init curl
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = {0};
    CURLcode res;

    //setup message
    string message = (string)malloc(sizeof(char)*1024);
    printf("Enter message: ");scanf("%s",message);
    string jsond = (string)malloc(sizeof(char)*1024);
    sprintf(jsond, "{\"content\":\"%s\"}", message);

    //setup request & headers
    headers = curl_slist_append(headers, "content-type: application/json"); //header
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsond);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_POST,        1L);

    //execute
    res = curl_easy_perform(curl);

    //error checks
    if (res != CURLE_OK)
        printf("[!] fail: %s", curl_easy_strerror(res));

    free(message);
    free(jsond);
}
void SendMessageToWithJSONWebHook(string URL)
{
    //init curl
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = {0};
    CURLcode res;

    //open file and read the content (must be valid json file)

    string loc = (string)malloc(sizeof(char)*4096);
    printf("Enter file location: "); scanf("%s", loc);
    FILE *json = fopen(loc, "r");
    size_t fsize = GetFileSize(json);
    string filedata = (string)malloc(sizeof(char)*fsize);
    fread(filedata, fsize, 1, json);
    fclose(json);

    //setup request & headers
    headers = curl_slist_append(headers, "content-type: application/json"); //header
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, filedata);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_POST,        1L);

    //execute
    res = curl_easy_perform(curl);

    //error checks
    if (res != CURLE_OK)
        printf("[!] fail: %s", curl_easy_strerror(res));
}
size_t GetFileSize(FILE *file)
{
    fseek(file, 0L, SEEK_END);
    size_t s = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return s;
}