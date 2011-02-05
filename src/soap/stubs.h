//gsoap ns service name: DarxenService
//gsoap ns service port: http://192.168.1.2:4888/DarxenService
// gsoap ns service style: rpc
// gsoap ns service encoding: encoded 
// gsoap ns service namespace: urn:DarxenService

typedef double xsd__double;
typedef char* xsd__string;
typedef char* xsd__base64Binary;
typedef enum Bool xsd__boolean;

enum Bool
{
	false_,
	true_
};

struct StringArray
{
	int __size;
	xsd__string* array;
};

struct RadarSiteInfo
{
	char* site;
	double lat;
	double lon;
	char* state;
	char* city;
};

struct Date
{
	int year;
	int month;
	int day;
};

struct Time
{
	int hour;
	int minute;
};

struct DateTime
{
	struct Date date;
	struct Time time;
};

struct RadarData
{
	xsd__base64Binary data;
};

struct RadarDataInfo
{
	xsd__string site;
	xsd__string product;
	struct DateTime date;
};

struct DateTimeArray
{
	int __size;
	struct DateTime* array;
};

struct RadarDataInfoArray
{
	int __size;
	struct RadarDataInfo* array;
};

int ns__hello_server(int* result);
int ns__goodbye_server(int sessionID, void* result);

int ns__add_radar_poller(int sessionID, char* site, char* product, xsd__boolean* result);
int ns__remove_radar_poller(int sessionID, char* site, char* product, xsd__boolean* result);
int ns__poller_data_get_queue_count(int sessionID, int* result);
int ns__poller_data_read_queue(int sessionID, int count, struct RadarDataInfoArray* result);

int ns__get_radar_data(int sessionID, char* site, char* product, struct DateTime time, struct RadarData* result);

int ns__search_radar_data(int sessionID, char* site, char* product, struct DateTime* start, struct DateTime* end, int* result); //0: no results, >0: searchID
int ns__search_get_count(int sessionID, int searchID, int* result);
int ns__search_get_records(int sessionID, int searchID, int start, int count, struct DateTimeArray* result);
int ns__search_free(int sessionID, int searchID, xsd__boolean* result);

/*
Notes:
.NET likes the int &result better than int* result, but c blows up
returning structs are problematic
*/


