/**
 * @file HelperFunctions.h
 * @author Rekkab S Gill 1009210
 * @date Feb 6 2019
 * @brief File contains the function declartions for added functions for A1
 */

#ifndef _HELP_API_
#define _HELP_API_

Property *initializeProperty(char* contentLine);
Alarm *initializeAlarm(void);
Event *initializeEvent(void);
void cleanUp(Event *calEvent,Alarm *calAlarm, Property *calProperty, Property *eventProperty, Property *alarmProperty);
char* createJSONCal(char* fileName);
char* getEventList(char* fileName);
char* alarmToJSON(const Alarm* alarm);
char* alarmListToJSON(const List* alarmList);
char* getAlarmList(char* fileName, char* theEvent);
char* getAlarmListForDB(char* fileName, char* theEvent);
char* fakeDtToJSON(DateTime prop);
//char* fakeDtToJSONforDB(DateTime prop);
char* propToJSON(const Property* theProp);
char* propListToJSON(const List* propList);
char* getPropList(char* fileName, char* theEvent);
char* getPropListForDB(char* fileName, char* theEvent);
char* fakeEventToJSON(const Event* event);
//char* fakeEventToJSONforDB(const Event* event);
char* makeCalFile(char* fileName, char *jsonCal, char *jsonEvent, char* createDate, char* createTime,char* createUTC,
                  char* startDate, char* startTime, char* startUTC, char* initialSum);

char* addEventToCal(char* fileName, char *jsonEvent, char* createDate, char* createTime,char* createUTC,
                    char* startDate, char* startTime, char* startUTC, char* addSum);


#endif
