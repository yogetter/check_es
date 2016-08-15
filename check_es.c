#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <string.h>
using namespace std;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void print_help(char *arg){
  string exec = arg;
  exec = exec.substr(2, exec.length());
  cout << "\nHelp for "<< exec << "\n" << endl;
  cout << "Basic usage: "<< exec <<" -H {Host IP of elasticsearch} -c {critical} -w {warning} \n" << endl;
  cout << "Command switches are optional, default values for warning is 2 and critical is 1" << endl;
  cout << "-w - Sets warning value for number of elasticsearch node. Default is 1" << endl;
  cout << "-c - Sets critical value for number of elasticsearch node. Default is 0" << endl;
  cout << "-h - Displays this help message\n" << endl;
  cout << "Example: ./" << exec << " -H 192.168.0.1:9200 -w 4 -c 2\n" << endl;
  cout << "Author: Getter Yang, getter.y@inwinstack.com" << endl;
}

bool get_parameter(int argc, char *argv[], string *host, int *warn, int *critical){
	for(int i = 1; i < argc; i=i+2){
        	char *arg = argv[i];
		string tmp;
        	switch (arg[1]){
			case 'H':
				*host = argv[i+1];
				break;
			case 'c':
                                tmp = argv[i+1];
				*critical = stoi(tmp);
				break;
			case 'w':
                                tmp = argv[i+1];
				*warn = stoi(tmp);
				break;
			default:
				cout << "Parameter Error!!!" << endl;
				print_help(argv[0]);
				return false;	
        	}
        }
	return true;
}

int main(int argc, char *argv[])
{	
	string host;
	int warn = 1, critical = 0; // Default value

	if ( argc == 1 || argc > 7){
		print_help(argv[0]);
		return 0;
	}
	else {
		if(!get_parameter(argc, argv, &host, &warn, &critical)){
			return 0;
		}
	}
        string url = "http://" + host + "/_cluster/health";
	CURL *curl;
	CURLcode res;
	string readBuffer;
	curl = curl_easy_init();
	if(curl){
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);

                string node = "\"number_of_nodes\"";
		size_t pos = readBuffer.find(node);
		int node_num = readBuffer[(int)pos+18] - '0';

                string status[] = {"\"status\":\"green\"", "\"status\":\"yellow\"", "\"status\":\"red\""};
                if( readBuffer.find(status[2]) != string::npos || node_num <= critical){
			cout << "elasticsearch CRITICAL: " << node_num << " elasticsearch nodes are running" << endl;
			return 2; 
		}
		else if ( readBuffer.find(status[1]) != string::npos || node_num <= warn ){
		        cout << "elasticsearch WARNING: " << node_num << " elasticsearch nodes are running" << endl;
			return 1;
		}
		else if( readBuffer.find(status[0]) != string::npos || node_num > warn ){
                        cout << "elasticsearch OK: " << node_num << " elasticsearch nodes are running" << endl;
			return 0;
		}
		else{
                        cout << "elasticsearch UNKNOWN: " << node_num <<" elasticsearch nodes are running" << endl;
			return 3;
		}

	}
}
