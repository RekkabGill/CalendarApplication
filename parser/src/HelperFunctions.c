/**
 * @file HelperFunctions.c
 * @author Rekkab S Gill (rekkab@uoguelph.ca)
 * @date Feb 4 2019
 * @brief Helper Functions for Calendar Parser
 */
#include "CalendarParser.h"
#include "LinkedListAPI.h"
#include "HelperFunctions.h"

//********************Calendar Helper Functions********************************
Property *initializeProperty(char* contentLine)
{
  Property *calProp = NULL;

  calProp = malloc(sizeof(Property) + strlen(contentLine) * sizeof(char));
  memset(calProp->propName,0,sizeof(calProp->propName));
  return calProp;
}

Alarm *initializeAlarm(void)
{
  Alarm *calAlarm = malloc(sizeof(Alarm));

  memset(calAlarm->action,0,sizeof(calAlarm->action));
  calAlarm->trigger = malloc(1000* sizeof(char));
  calAlarm->properties = initializeList(printAlarm,deleteAlarm,compareAlarms);

  return calAlarm;
}

Event *initializeEvent(void)
{
  Event *calEvent = malloc(sizeof(Event));

  memset(calEvent->UID,0,sizeof(calEvent->UID));
  memset(calEvent->creationDateTime.date,0,sizeof(calEvent->creationDateTime.date));
  memset(calEvent->creationDateTime.time,0,sizeof(calEvent->creationDateTime.time));
  calEvent->creationDateTime.UTC = false;

  memset(calEvent->startDateTime.date,0,sizeof(calEvent->startDateTime.date));
  memset(calEvent->startDateTime.time,0,sizeof(calEvent->startDateTime.time));
  calEvent->startDateTime.UTC = false;

  calEvent->properties = initializeList(printProperty,deleteProperty,compareProperties);
  calEvent->alarms = initializeList(printAlarm,deleteAlarm,compareAlarms);

  return calEvent;
}

void cleanUp(Event *calEvent,Alarm *calAlarm, Property *calProperty, Property *eventProperty, Property *alarmProperty)
{
  if(alarmProperty != NULL)
  {
    free(alarmProperty);
  }
  if(eventProperty != NULL)
  {
    free(eventProperty);
  }
  if(calProperty != NULL)
  {
    free(calProperty);
  }
  if(calAlarm != NULL)
  {
    if(calAlarm->properties != NULL)
    {
      freeList(calAlarm->properties);
    }
    free(calAlarm->trigger);
    free(calAlarm);
  }
  if(calEvent != NULL)
  {
    if(calEvent->properties != NULL)
    {
      freeList(calEvent->properties);
    }
    if(calEvent->alarms != NULL)
    {
      freeList(calEvent->alarms);
    }
    free(calEvent);
  }
}


//********************Property Helper Functions********************************
void deleteProperty(void* toBeDeleted)
{
  if(toBeDeleted != NULL)
  {
    free(toBeDeleted);
  }
}
int compareProperties(const void* first, const void* second)
{
  if(first == NULL || second == NULL)
  {
    return -1;
  }
  const Property *propertyOne = first;
  const Property *propertyTwo = second;

  if( (strcmp(propertyOne->propName, propertyTwo->propName) == 0) &&
    (strcmp(propertyOne->propDescr,propertyTwo->propDescr) == 0) )
  {
    return 0;
  }
  else
  {
    return 1;
  }


}
char *printProperty(void* toBePrinted)
{
  char *printedProperty = NULL;
  Property *propertyObject = toBePrinted;

  if(toBePrinted == NULL)
  {
    return NULL;
  }
  printedProperty = malloc(1000 * sizeof(char));

  strcpy(printedProperty,propertyObject->propName);
  strcat(printedProperty,":");
  strcat(printedProperty,propertyObject->propDescr);

  return printedProperty;
}


//********************Alarm Helper Functions********************************
void deleteAlarm(void* toBeDeleted)
{
  free(toBeDeleted);
}

int compareAlarms(const void* first, const void* second)
{
  if(first == NULL || second == NULL)
  {
    return -1;
  }

  const Alarm *alarmOne = first;
  const Alarm *alarmTwo = second;

  if( (strcmp(alarmOne->action,alarmTwo->action) == 0) )
  {
    return 0;
  }
  else
  {
    return 1;
  }

}
char* printAlarm(void* toBePrinted)
{
  char* printedAlarm = NULL;
  Alarm *alarmObject = toBePrinted;

  if(toBePrinted == NULL)
  {
    return NULL;
  }

  printedAlarm = malloc(1000 * sizeof(char));

  strcpy(printedAlarm, alarmObject->action);
  strcat(printedAlarm,";");
  strcat(printedAlarm, alarmObject->trigger);

  return printedAlarm;
}

//********************Event Helper Functions********************************
void deleteEvent(void* toBeDeleted)
{
  free(toBeDeleted);
}

int compareEvents(const void* first, const void* second)
{
  if(first == NULL || second == NULL)
  {
    return -1;
  }

  const Event *eventOne = first;
  const Event *eventTwo = second;

  if(strcmp(eventOne->UID,eventTwo->UID) == 0)
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

char* printEvent(void* toBePrinted)
{
  char *printedEvent = NULL;
  Event *eventObject = toBePrinted;

  if(toBePrinted == NULL)
  {
    return NULL;
  }

  printedEvent = malloc(1000* sizeof(char));
  strcpy(printedEvent,eventObject->UID);

  return printedEvent;
}

//********************Date Helper Functions ********************************
void deleteDate(void* toBeDeleted)
{
  free(toBeDeleted);
}

int compareDates(const void* first, const void* second)
{
  return 0;
}

char* printDate(void* toBePrinted)
{
  char* printedDate = NULL;
  DateTime *dateObject = toBePrinted;

  if(toBePrinted == NULL)
  {
    return NULL;
  }

  printedDate = malloc(1000 * sizeof(char));

  if(dateObject->UTC == true)
  {
    snprintf(printedDate,999,"Date:%s, Time:%s, UTC: true",dateObject->date,dateObject->time);
  }
  else
  {
      snprintf(printedDate,999,"Date:%s, Time:%s, UTC: false",dateObject->date,dateObject->time);
  }

  return printedDate;
}
/**************************************WRAPPER FUNCTIONS**************************************************/
char* createJSONCal(char* fileName)
{
  Calendar *myCalendar = NULL;
  char *jsonCal = NULL;

  myCalendar = malloc(sizeof(myCalendar));

  createCalendar(fileName, &myCalendar);
  jsonCal = calendarToJSON(myCalendar);
  free(myCalendar);
  return jsonCal;
}

char* getEventList(char* fileName)
{
  Calendar *myCalendar = NULL;
  char *eventListJSON = NULL;

  myCalendar = malloc(sizeof(myCalendar));
  createCalendar(fileName, &myCalendar);
  eventListJSON = eventListToJSON(myCalendar->events);
  free(myCalendar);
  return eventListJSON;
}

char* makeCalFile(char* fileName, char *jsonCal, char *jsonEvent, char* createDate, char* createTime,char* createUTC,
                  char* startDate, char* startTime, char* startUTC, char* initialSum)
{
  Calendar *myCalendar = NULL;
  Event *newEvent = NULL;
  ICalErrorCode errorCode;
  Property* summaryProperty = NULL;

  newEvent = JSONtoEvent(jsonEvent);
  myCalendar = JSONtoCalendar(jsonCal);

  //setup the date and time for the new event
  strcpy(newEvent->startDateTime.date,startDate);
  strcpy(newEvent->startDateTime.time,startTime);
  if(strcmp(startUTC,"true") == 0)
  {
    newEvent->startDateTime.UTC = true;
  }
  else
  {
    newEvent->startDateTime.UTC = false;
  }
  strcpy(newEvent->creationDateTime.date,createDate);
  strcpy(newEvent->creationDateTime.time,createTime);
  if(strcmp(createUTC,"true") == 0)
  {
    newEvent->creationDateTime.UTC = true;
  }
  else
  {
    newEvent->creationDateTime.UTC = false;
  }

  if(strlen(initialSum) > 0)
  {
    summaryProperty = malloc(sizeof(Property));
    strcpy(summaryProperty->propName,"SUMMARY");
    strcpy(summaryProperty->propDescr,initialSum);
    insertBack(newEvent->properties,summaryProperty);
  }
  //add event to calendar
  addEvent(myCalendar,newEvent);
  errorCode = validateCalendar(myCalendar);

  if(errorCode == OK)
  {
    errorCode = writeCalendar(fileName,myCalendar);
    deleteCalendar(myCalendar);
    return printError(errorCode);
  }
  else
  {
    free(myCalendar);
    free(newEvent);
    return printError(errorCode);
  }
}
char* addEventToCal(char* fileName, char *jsonEvent, char* createDate, char* createTime,char* createUTC,
                    char* startDate, char* startTime, char* startUTC, char* addSum)
{
  Calendar *myCalendar = NULL;
  Event *newEvent = NULL;
  ICalErrorCode errorCode;
  Property* summaryProperty = NULL;

  newEvent = JSONtoEvent(jsonEvent);
  errorCode = createCalendar(fileName,&myCalendar);

  if(errorCode == OK)
  {
    //setup the date and time for the new event
    strcpy(newEvent->startDateTime.date,startDate);
    strcpy(newEvent->startDateTime.time,startTime);

    if(strcmp(startUTC,"true") == 0)
    {
      newEvent->startDateTime.UTC = true;
    }
    else
    {
      newEvent->startDateTime.UTC = false;
    }

    strcpy(newEvent->creationDateTime.date,createDate);
    strcpy(newEvent->creationDateTime.time,createTime);

    if(strcmp(createUTC,"true") == 0)
    {
      newEvent->creationDateTime.UTC = true;
    }
    else
    {
      newEvent->creationDateTime.UTC = false;
    }
    if(strlen(addSum) > 0)
    {
      summaryProperty = malloc(sizeof(Property));
      strcpy(summaryProperty->propName,"SUMMARY");
      strcpy(summaryProperty->propDescr,addSum);
      insertBack(newEvent->properties,summaryProperty);
    }
    //add event to calendar
    addEvent(myCalendar,newEvent);
    errorCode = validateCalendar(myCalendar);

    if(errorCode == OK)
    {
      errorCode = writeCalendar(fileName,myCalendar);
      deleteCalendar(myCalendar);
      return printError(errorCode);
    }
    else
    {
      deleteCalendar(myCalendar);
      return printError(errorCode);
    }
  }
  else
  {
    free(newEvent);
    return printError(errorCode);
  }
}

char* alarmToJSON(const Alarm* alarm)
{
  char *jsonAlarm = NULL;
  int numOfProps = 0;

  jsonAlarm = calloc(10000, sizeof(char));

  if(alarm == NULL)
  {
    snprintf(jsonAlarm,9999,"{}");
    return jsonAlarm;
  }

  numOfProps = 2 + (getLength(alarm->properties));

  if(numOfProps < 2)
  {
    snprintf(jsonAlarm,9999,"{}");
    return jsonAlarm;
  }

  snprintf(jsonAlarm,9999,"{\"action\":\"%s\",\"trigger\":\"%s\",\"numProps\":%d}",alarm->action,alarm->trigger,numOfProps);


  return jsonAlarm;
}

char* alarmListToJSON(const List* alarmList)
{
  char* jsonAlarmList = NULL;
  char* jsonAlarm = NULL;
  ListIterator alarmIter;
  Alarm *currentAlarm = NULL;
  int alarmListLength = 0;
  int alarmCount = 1;

  jsonAlarmList = calloc(10000, sizeof(char));

  if(alarmList == NULL)
  {
    snprintf(jsonAlarmList,9999,"[]");
    return jsonAlarmList;
  }

  alarmListLength = getLength((List*)alarmList);
  strcat(jsonAlarmList,"[");

  alarmIter = createIterator((List*)alarmList);
  while(1)
  {
    currentAlarm = nextElement(&alarmIter);

    if(currentAlarm == NULL)
    {
      break;
    }

    jsonAlarm = alarmToJSON(currentAlarm);
    strcat(jsonAlarmList,jsonAlarm);

    if(alarmCount < alarmListLength)
    {
      strcat(jsonAlarmList,",");
    }
    alarmCount++;
    free(jsonAlarm);
  }
  strcat(jsonAlarmList,"]");

  return jsonAlarmList;
}


char* getAlarmList(char* fileName, char* theEvent)
{
  Calendar *myCalendar = NULL;
  ListIterator eventIter;
  Event *tempEvent = NULL;
  char *tempEventJSON = NULL;
  char *alarmListJSON = NULL;


  myCalendar = malloc(sizeof(myCalendar));
  createCalendar(fileName, &myCalendar);

  eventIter = createIterator(myCalendar->events);
  while(1)
  {
    tempEvent = nextElement(&eventIter);

    if(tempEvent == NULL)
    {
      break;
    }
    tempEventJSON = fakeEventToJSON(tempEvent);

    if(strcmp(tempEventJSON,theEvent) == 0)
    {
      alarmListJSON = alarmListToJSON(tempEvent->alarms);
      free(tempEventJSON);
      deleteCalendar(myCalendar);
      return alarmListJSON;
    }
    free(tempEventJSON);
  }
  deleteCalendar(myCalendar);
  return NULL;
}

char* fakeEventToJSON(const Event* event)
{
  char *jsonEvent = NULL;
  char *jsonDT = NULL;
  int numOfProps = 0;
  int numOfAlarms = 0;
  ListIterator eventPropertyIter;
  Property *eventProperty;

  jsonEvent = calloc(10000, sizeof(char));

  if(event == NULL)
  {
    snprintf(jsonEvent,9999,"{}");
    return jsonEvent;
  }

  jsonDT = fakeDtToJSON(event->startDateTime);
  numOfProps = 3 + (getLength(event->properties));
  numOfAlarms = getLength(event->alarms);

  if(numOfProps < 3)
  {
    snprintf(jsonEvent,9999,"{}");
    return jsonEvent;
  }

  eventPropertyIter = createIterator(event->properties);
  while(1)
  {
    eventProperty = nextElement(&eventPropertyIter);

    if(eventProperty == NULL)
    {
      break;
    }
    else if(strcmp(eventProperty->propName,"SUMMARY") == 0)
    {
      break;
    }
  }

  if(eventProperty == NULL)
  {
    snprintf(jsonEvent,9999,"{\"startDT\":%s,\"numProps\":\"%d\",\"numAlarms\":\"%d\",\"summary\":\"\"}",
            jsonDT,numOfProps,numOfAlarms);
  }
  else
  {
    snprintf(jsonEvent,9999,"{\"startDT\":%s,\"numProps\":\"%d\",\"numAlarms\":\"%d\",\"summary\":\"%s\"}",
            jsonDT,numOfProps,numOfAlarms,eventProperty->propDescr);
  }

  free(jsonDT);
  return jsonEvent;
}

char* fakeDtToJSON(DateTime prop)
{
  char *jsonDT = NULL;

  jsonDT = calloc(1000, sizeof(char));

  if(prop.UTC == true)
  {
    snprintf(jsonDT,999,"{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":\"true\"}",prop.date,prop.time);
  }
  else
  {
    snprintf(jsonDT,999,"{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":\"false\"}",prop.date,prop.time);
  }

  return jsonDT;
}

char* getPropList(char* fileName, char* theEvent)
{
  Calendar *myCalendar = NULL;
  ListIterator eventIter;
  Event *tempEvent = NULL;
  char *tempEventJSON = NULL;
  char *propListJSON = NULL;


  myCalendar = malloc(sizeof(myCalendar));
  createCalendar(fileName, &myCalendar);

  eventIter = createIterator(myCalendar->events);
  while(1)
  {
    tempEvent = nextElement(&eventIter);

    if(tempEvent == NULL)
    {
      break;
    }
    tempEventJSON = fakeEventToJSON(tempEvent);

    if(strcmp(tempEventJSON,theEvent) == 0)
    {
      propListJSON = propListToJSON(tempEvent->properties);
      free(tempEventJSON);
      deleteCalendar(myCalendar);
      return propListJSON;
    }
    free(tempEventJSON);
  }
  deleteCalendar(myCalendar);
  return NULL;
}

char* propListToJSON(const List* propList)
{
  char* jsonPropList = NULL;
  char* jsonProp = NULL;
  ListIterator propIter;
  Property *currentProp = NULL;
  int propListLength = 0;
  int propCount = 1;

  jsonPropList = calloc(10000, sizeof(char));

  if(propList == NULL)
  {
    snprintf(jsonPropList,9999,"[]");
    return jsonPropList;
  }

  propListLength = getLength((List*)propList);
  strcat(jsonPropList,"[");

  propIter = createIterator((List*)propList);
  while(1)
  {
    currentProp = nextElement(&propIter);

    if(currentProp == NULL)
    {
      break;
    }

    jsonProp = propToJSON(currentProp);
    strcat(jsonPropList,jsonProp);

    if(propCount < propListLength)
    {
      strcat(jsonPropList,",");
    }
    propCount++;
    free(jsonProp);
  }
  strcat(jsonPropList,"]");

  return jsonPropList;
}

char* propToJSON(const Property* theProp)
{
  char *jsonProp = NULL;

  jsonProp = calloc(10000, sizeof(char));

  if(theProp == NULL)
  {
    snprintf(jsonProp,9999,"{}");
    return jsonProp;
  }

  snprintf(jsonProp,9999,"{\"propName\":\"%s\",\"propDescr\":\"%s\"}",theProp->propName,theProp->propDescr);
  return jsonProp;
}

char* getPropListForDB(char* fileName, char* theEvent)
{
  Calendar *myCalendar = NULL;
  ListIterator eventIter;
  Event *tempEvent = NULL;
  char *tempEventJSON = NULL;
  char *propListJSON = NULL;


  myCalendar = malloc(sizeof(myCalendar));
  createCalendar(fileName, &myCalendar);

  eventIter = createIterator(myCalendar->events);
  while(1)
  {
    tempEvent = nextElement(&eventIter);

    if(tempEvent == NULL)
    {
      break;
    }
    tempEventJSON = eventToJSON(tempEvent);

    if(strcmp(tempEventJSON,theEvent) == 0)
    {
      propListJSON = propListToJSON(tempEvent->properties);
      free(tempEventJSON);
      deleteCalendar(myCalendar);
      return propListJSON;
    }
    free(tempEventJSON);
  }
  deleteCalendar(myCalendar);
  return NULL;
}

char* getAlarmListForDB(char* fileName, char* theEvent)
{
  Calendar *myCalendar = NULL;
  ListIterator eventIter;
  Event *tempEvent = NULL;
  char *tempEventJSON = NULL;
  char *alarmListJSON = NULL;


  myCalendar = malloc(sizeof(myCalendar));
  createCalendar(fileName, &myCalendar);

  eventIter = createIterator(myCalendar->events);
  while(1)
  {
    tempEvent = nextElement(&eventIter);

    if(tempEvent == NULL)
    {
      break;
    }
    tempEventJSON = eventToJSON(tempEvent);

    if(strcmp(tempEventJSON,theEvent) == 0)
    {
      alarmListJSON = alarmListToJSON(tempEvent->alarms);
      free(tempEventJSON);
      deleteCalendar(myCalendar);
      return alarmListJSON;
    }
    free(tempEventJSON);
  }
  deleteCalendar(myCalendar);
  return NULL;
}
