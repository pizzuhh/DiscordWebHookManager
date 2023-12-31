#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
void DeleteWebHook(char*);
void SendMessageToWebHook(char*);
void SendMessageToWithJSONWebHook(char*);
int CheckAndDisplayWebhookInfo(char*);
void MM(char*);
size_t GetFileSize(FILE*);

int main(void)
{
    char* URL = (char*)malloc(sizeof(char)*1024); //store the URL
    char* webhookchk = "https://discord.com/api/webhooks"; //size and webhook link for checking
    size_t strwhlen = strlen(webhookchk);

    //try to open "load" file if the file don't exist or can't be opened switch to manual
    FILE* load = fopen("load", "r");

    if(!load)
    {
        printf("[?] file \"load\" doesn't exist or can't be opened!\n");
        printf("Enter webhook URL: ");
        fgets(URL, 1024, stdin);
        URL[strlen(URL)-1] = '\0';
        if(CheckAndDisplayWebhookInfo(URL))
        {
            printf("[!]Error couldn't fetch webhook info! Check if the link is valid!\n");
            //free(URL);
            return 1;
        }
        MM(URL);
        //free(URL);
        return 0;
    }

    //if the file exists read the URL from it
    size_t siz = GetFileSize(load);
    fread(URL, siz, 1, load);
    //check if webhook URL is valid
    if(URL[(strlen(URL))-1] == '\n')
        URL[(strlen(URL))-1] = '\0';
    fclose(load);
    if(!strncmp(URL, webhookchk, strwhlen))
    {
        if(CheckAndDisplayWebhookInfo(URL))
        {
            printf("[!]Error couldn't fetch webhook info! Check if the link is valid!\n");
            free(URL);
            return 1;
        }
        printf("\n\n[*]Webhook URL: %s\n\n", URL);
        MM(URL);
        free(URL);
        return 0;
    }

    //if the webhook check fail continue
    printf("[?] loading URL from file failed! (invalid URL)\n");
    printf("Enter webhook URL: ");
    fgets(URL, 1024, stdin);

    URL[strlen(URL)-1] = '\0';
    if(CheckAndDisplayWebhookInfo(URL))
    {
        printf("[!]Error couldn't fetch webhook info! Check if the link is valid!\n");
        return 1;
    }
    MM(URL);
    return 0;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    cJSON *cJson, *id, *channel_id, *guild_id, *name, *token, *avatar, *user, *user_name, *user_displayname, *user_id;
    char av[1024*3];
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
    printf
            (
                    "----------INFO------------\n"
                    "name: %s\n"
                    "id: %s\n"
                    "guild_id: %s\n"
                    "token: %s\n"
                    "channel id %s\n"
                    "avatar url: %s\n"
                    "user name: %s\n"
                    "user id: %s\n--------------------------\n",
                    name->valuestring, id->valuestring, guild_id->valuestring, token->valuestring, channel_id->valuestring, av, user_name->valuestring, user_id->valuestring
            );
    cJSON_free(cJson);cJSON_free(id);cJSON_free(channel_id);cJSON_free(guild_id);cJSON_free(name);cJSON_free(token);cJSON_free(avatar);cJSON_free(user);cJSON_free(user_name);
    cJSON_free(user_displayname);cJSON_free(user_id);
    return nmemb;
}
int CheckAndDisplayWebhookInfo(char* URL)
{
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = {0};
    CURLcode res;

    headers = curl_slist_append(headers, "content-type: application/json"); //header
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if(res != CURLE_OK)
    {
        return 1;
    }
    return 0;

}
void MM(char* URL)
{
    putc('\n', stdout);
    int c;
    printf("1. Send Message\n2. Send message by loading JSON file\n3. Delete WebHook[Danger]\n4. Exit\n"); c = fgetc(stdin);
    switch (c) {
        case 49:
            SendMessageToWebHook(URL);
            break;
        case 50:
            SendMessageToWithJSONWebHook(URL);
            break;
        case 51:
            DeleteWebHook(URL);
            break;
        default:
            break;
    }
}
void DeleteWebHook(char* URL)
{
    //init curl
    CURL *curl = curl_easy_init();
    CURLcode res;

    //set up the request
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_URL, URL);

    //execute
    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    //error check
    if(res != CURLE_OK)
    {
        printf("[!] fail: %s", curl_easy_strerror(res));
        exit(1);
    }
    printf("Webhook deleted!\n");

}
void SendMessageToWebHook(char* URL)
{
    //init curl
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = {0};

    CURLcode res;

    //setup message
    printf("Enter message: ");

    char* message = (char*)malloc(sizeof(char)*2000);

    fgetc(stdin);
    fgets(message, 2000, stdin);

    message[strlen(message)-1] = '\0';

    char* jsond = (char*)malloc(sizeof(char)*1024);
    sprintf(jsond, "{\"content\":\"%s\"}", message);

    //setup request & headers
    headers = curl_slist_append(headers, "content-type: application/json"); //header
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsond);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_POST,        1L);



    //execute
    res = curl_easy_perform(curl);
    free(message);
    free(jsond);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    //error checks
    if (res != CURLE_OK)
    {
        printf("[!] fail: %s", curl_easy_strerror(res));
        exit(1);
    }
    printf("Message sent!\n");
}
void SendMessageToWithJSONWebHook(char* URL)
{
    //init curl
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = {0};
    CURLcode res;

    //open file and read the content (must be valid json file)

    char* loc = (char*)malloc(sizeof(char)*4096);

    //dispaly .json files if there are any in '.'
    printf("--JSON files found--\n");
    struct dirent *ent;
    DIR *dir = opendir(".");
    if(!dir)
    {
        printf("Cannont open current folder!\n");
    }

    while((ent = readdir(dir)))
    {
        if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
        {
            char* dot = strrchr(ent->d_name, '.');
            if(dot && !strcmp(dot, ".json"))
            {
                printf("%s\n", ent->d_name); //TODO: improve this if file has more than one '.'
            }
        }
    }
    printf("--------------------\n");

    printf("Enter file location: ");
    getc(stdin);
    fgets(loc, 4096, stdin);
    loc[strlen(loc) - 1] = '\0';

    FILE *json = fopen(loc, "r");
    if(!json)
    {
        printf("File doesn't exists");
        exit(1);
    }

    size_t fsize = GetFileSize(json);
    char* filedata = (char*)malloc(sizeof(char)*fsize);



    fread(filedata, fsize, 1, json);
    fclose(json);
    free(loc);
    //clean the JSON data
    for (int i = 0; i < strlen(filedata); ++i)
    {
        if(filedata[i] == '\x0')
        {
            filedata[i] = 'x';
        }
    }

    //setup request & headers
    headers = curl_slist_append(headers, "content-type: application/json"); //header
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, filedata);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_POST,        1L);




    //execute
    res = curl_easy_perform(curl);
    free(filedata);
    //error checks
    if (res != CURLE_OK)
    {
        printf("[!] fail: %s", curl_easy_strerror(res));
        exit(1);
    }
    printf("Message sent!\n");
}
size_t GetFileSize(FILE *file)
{
    fseek(file, 0L, SEEK_END);
    size_t s = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return s;
}
