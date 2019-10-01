
#include <stdio.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

int main(int argc, char **argv) {
  //inicio declaramos vars,ponteiros...
    CURL *curl_handle;
    char * msg;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    CURLcode saida;
  //iniciamos o spider
  //definimos status para nosso post no twitter 
    msg ="status=twitter post with C Code by Cooler_";
  //passamos URL
    curl_easy_setopt(curl_handle, CURLOPT_URL, "http://twitter.com/statuses/update.json");
//    curl_easy_setopt(curl_handle, CURLOPT_RETURNTRANSFER, 1); 
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 2);
    curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, msg);
  //definimos campos de login e senha
    curl_easy_setopt(curl_handle, CURLOPT_USERPWD, "Cooler_freenode:1234");
  //envia dados
    curl_easy_perform(curl_handle);
 //vishu nayak//
  //limpa tudo
    curl_easy_cleanup(curl_handle);
 return (0);
 
}
