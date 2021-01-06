/**
 * @file CalendarParser.c
 * @author Rekkab S Gill (rekkab@uoguelph.ca)
 * @date Feb 4 2019
 * @brief Calendar Library
 */
 #include "CalendarParser.h"
 #include "HelperFunctions.h"
 #include <unistd.h>
 #include <ctype.h>
 #include <stdbool.h>

ICalErrorCode createCalendar(char* fileName, Calendar** obj)
{
  Calendar *tempCal = NULL;
  Event *calEvent = NULL;
  Alarm *calAlarm = NULL;
  Property *calProperty = NULL;
  Property *eventProperty = NULL;
  Property *alarmProperty = NULL;
  FILE *calendarFile =  NULL;
  char *extensionName = NULL;
  char currentLineRead[2000] = {0};
  int parserIndex = 0;
  char delimToken[3] = ":;";
  char timeToken[2] = "T";
  char *stringPointer = NULL;
  char contentLines[100][2000] = {{0}};
  int contentLineCount = 0;
  int i = 0;
  char tempVar = 0;
  int nextChar = 0;
  int numOfStrings = 0;
  char tempArray[2000] = {0};
  bool alarmFlag = false;
  bool eventFlag = false;
  char utcBuffer[100] = "";


  //malloc for calendar
  tempCal = malloc(sizeof(Calendar));
  tempCal->version = 0.0;
  memset(tempCal->prodID,'\0',sizeof(tempCal->prodID));
  tempCal->events = initializeList(printEvent,deleteEvent,compareEvents);
  tempCal->properties = initializeList(printProperty, deleteProperty, compareProperties);

  //check if the file name, extension are correct and if it exists
  if(fileName == NULL)
  {
    deleteCalendar(tempCal);
    cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
    *obj = NULL;
    return INV_FILE;
  }
  extensionName = strrchr(fileName,'.');
  if(strcmp(extensionName,".ics") != 0 || strlen(fileName) == 0)
  {
    deleteCalendar(tempCal);
    cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
    *obj = NULL;
    return INV_FILE;
  }
  else if(access(fileName, F_OK) == -1 || access(fileName, R_OK) == -1 )
  {
    deleteCalendar(tempCal);
    cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
    *obj = NULL;
    return INV_FILE;
  }

  calendarFile = fopen(fileName, "r");

  /* The following loop parses each character into an array, then each character is copied
  * into a 2D array of strings in which each string is a content line*/
  while(1)
  {
   if(feof(calendarFile))
   {
     break;
   }
   //holds the first char of new content line due to previous contentline check
   if(tempVar != 0)
   {
     currentLineRead[parserIndex] = tempVar;
     parserIndex++;
     tempVar = 0;
   }
   //parse each character into an array
   currentLineRead[parserIndex] = fgetc(calendarFile);

   /* this block will parse the lines without including the newline or carriage*/
   //these if statements unfolds the folded line if a segment of '\r' '\n' ' ' is found
   if(parserIndex >= 2)
   {
     if(currentLineRead[parserIndex-2] == '\r' && currentLineRead[parserIndex-1] == '\n')
     {
       if((currentLineRead[parserIndex] == ' ') || currentLineRead[parserIndex] == '\t' )
       {
         parserIndex++;
         continue;
       }
       /* if a \r\n segment is found without a space char (which indicates a fold):
        *creates a new content line by parsing currentline into a new content line*/
       for(i = 0; i < (strlen(currentLineRead) -3); i++)
       {
         if(i >= 1)
         {
           //check to see if there is a single \r character in the line
           if(currentLineRead[i-1] == '\r'  && currentLineRead[i] != '\n')
           {
             contentLines[contentLineCount][nextChar] = '\r';
             nextChar++;
           }
           //check to see if there is a single \n character in the line
           else if(currentLineRead[i-1] != '\r'  && currentLineRead[i] == '\n')
           {
              contentLines[contentLineCount][nextChar] = '\n';
              nextChar++;
           }
         }
         if(currentLineRead[i] != '\r' && currentLineRead[i] != '\n')
         {
           if( i >= 2 && currentLineRead[i-2] == '\r' && currentLineRead[i-1] == '\n' &&
             (currentLineRead[i] == ' ' || currentLineRead[i] == '\t'))
           {
             continue;
           }
           contentLines[contentLineCount][nextChar] = currentLineRead[i];
           nextChar++;
         }
       }
       tempVar = currentLineRead[parserIndex];
       contentLines[contentLineCount][nextChar] = '\0';
       contentLineCount++;
       numOfStrings++;
       memset(currentLineRead,0,sizeof(currentLineRead));
       parserIndex = 0;
       nextChar = 0;
       continue;
     }
   }
   parserIndex++;
  }
  //*************************************PARSE CONTENT LINES INTO THE STRUCT*************************************************************************************************

  if(strcmp(contentLines[0],"BEGIN:VCALENDAR") != 0 ||
    strcmp(contentLines[numOfStrings-1],"END:VCALENDAR") != 0)
  {
   deleteCalendar(tempCal);
   cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
   fclose(calendarFile);
   return INV_CAL;
  }

  //Loop 2: Parse the Events;
  i = 1;
  while(1)
  {
    if(i >= numOfStrings)
    {
      break;
    }
    if(contentLines[i][0] == ';')
    {
      i++;
      continue;
    }

     memset(tempArray,0,sizeof(tempArray));
     strcpy(tempArray,contentLines[i]);
     stringPointer = strtok(tempArray,delimToken);

   if(strcmp(contentLines[i],"END:VCALENDAR") == 0)
   {
     break;
   }
   else if(strcmp(stringPointer,"VERSION") == 0)
   {
     if(tempCal->version != 0)
     {
       deleteCalendar(tempCal);
       cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
       fclose(calendarFile);
       return DUP_VER;
     }

     stringPointer = strtok(NULL,delimToken);

     if(stringPointer == NULL)
     {
       deleteCalendar(tempCal);
       cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
       fclose(calendarFile);
       return INV_VER;
     }
     if(atof(stringPointer) != 0)
     {
       tempCal->version = atof(stringPointer);
     }
     else
     {
       deleteCalendar(tempCal);
       cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
       fclose(calendarFile);
       return INV_VER;
     }
   }
   else if(strcmp(stringPointer,"PRODID") == 0)
   {
     if(strlen(tempCal->prodID) > 0)
     {
       deleteCalendar(tempCal);
       cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
       fclose(calendarFile);
       return DUP_PRODID;
     }

     stringPointer = strtok(NULL,"");

     if(stringPointer == NULL)
     {
       deleteCalendar(tempCal);
       cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
       fclose(calendarFile);
       return INV_PRODID;
     }

     if(strlen(stringPointer) > 0)
     {
       strcpy(tempCal->prodID,stringPointer);
     }
     else
     {
       deleteCalendar(tempCal);
       cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
       fclose(calendarFile);
       return INV_PRODID;
     }
   }
   else if(strcmp(contentLines[i],"BEGIN:VEVENT") == 0)
   {
     //this is an event within an event, so an error
     if(eventFlag == true)
     {
       deleteCalendar(tempCal);
       cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
       fclose(calendarFile);
       return INV_EVENT;
     }

     calEvent = initializeEvent();
     if(calEvent == NULL)
     {
       deleteCalendar(tempCal);
       cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
       fclose(calendarFile);
       return OTHER_ERROR;
     }

     eventFlag = true;
     i++;
     continue;
   }
   /*Handles the occurrence of an EVENT*/
   else if(eventFlag == true)
   {

     if(strcmp(stringPointer,"UID") == 0)
     {
       if(strlen(calEvent->UID) > 0)
       {
         deleteCalendar(tempCal);
         cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
         fclose(calendarFile);
         return INV_EVENT;
       }
       stringPointer = strtok(NULL,"");

       if(strlen(stringPointer) > 0)
       {
         strcpy(calEvent->UID,stringPointer);
       }
       else
       {
         deleteCalendar(tempCal);
         cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
         fclose(calendarFile);
         return INV_EVENT;
       }
     }
     else if(strcmp(stringPointer,"DTSTAMP") == 0)
     {
       if((strlen(calEvent->creationDateTime.date) > 0) || (strlen(calEvent->creationDateTime.time) > 0))
       {
         deleteCalendar(tempCal);
         cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
         fclose(calendarFile);
         return INV_DT;
       }

       stringPointer = strtok(NULL,timeToken);

       //missing time token or malformed
       if(strlen(stringPointer) != 8)
       {
         deleteCalendar(tempCal);
         cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
         fclose(calendarFile);
         return INV_DT;
       }

       strcpy(calEvent->creationDateTime.date,stringPointer);
       stringPointer = strtok(NULL,timeToken);
       if(strrchr(stringPointer,'Z') == NULL)
       {
         strcpy(calEvent->creationDateTime.time,stringPointer);
       }
       else
       {
         strcpy(utcBuffer,stringPointer);
         utcBuffer[strlen(stringPointer)-1] = '\0';
         strcpy(calEvent->creationDateTime.time,utcBuffer);
         calEvent->creationDateTime.UTC = true;
         memset(utcBuffer,0,sizeof(utcBuffer));
       }

       if((strlen(calEvent->creationDateTime.date) < 8) || (strlen(calEvent->creationDateTime.time) < 6))
       {
         deleteCalendar(tempCal);
         cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
         fclose(calendarFile);
         return INV_DT;
       }
     }
     else if(strcmp(stringPointer,"DTSTART") == 0)
     {
       if((strlen(calEvent->startDateTime.date) > 0) || (strlen(calEvent->startDateTime.time) > 0) )
       {
         deleteCalendar(tempCal);
         cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
         fclose(calendarFile);
         return INV_DT;
       }
       stringPointer = strtok(NULL,timeToken);

       //missing time token
       if(strlen(stringPointer) != 8)
       {
         deleteCalendar(tempCal);
         cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
         fclose(calendarFile);
         return INV_DT;
       }

       strcpy(calEvent->startDateTime.date,stringPointer);
       stringPointer = strtok(NULL,timeToken);

       if(strrchr(stringPointer,'Z') == NULL)
       {
         strcpy(calEvent->startDateTime.time,stringPointer);
       }
       else
       {
         strcpy(utcBuffer,stringPointer);
         utcBuffer[strlen(stringPointer)-1] = '\0';
         strcpy(calEvent->startDateTime.time,utcBuffer);
         calEvent->startDateTime.UTC = true;
         memset(utcBuffer,0,sizeof(utcBuffer));
       }

       if((strlen(calEvent->startDateTime.date) < 8) || (strlen(calEvent->startDateTime.time) < 6))
       {
         deleteCalendar(tempCal);
         cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
         fclose(calendarFile);
         return INV_DT;
       }
     }
     /*The following blocks of code will handle alarms within an event*/
     else if(strcmp(contentLines[i],"BEGIN:VALARM") == 0)
     {
       //this is an alarm within an alarm, so an error
       if(alarmFlag == true)
       {
         deleteCalendar(tempCal);
         cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
         fclose(calendarFile);
         return INV_ALARM;
       }
       calAlarm = initializeAlarm();

       if(calAlarm == NULL)
       {
         deleteCalendar(tempCal);
         cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
         fclose(calendarFile);
         return OTHER_ERROR;
       }

       alarmFlag = true;
       i++;
       continue;
     }
     //handles the occurence of an alarm
     else if(alarmFlag == true)
     {
        if(strcmp(stringPointer,"ACTION") == 0)
        {
          stringPointer = strtok(NULL,"");
          if(stringPointer == NULL)
          {
            deleteCalendar(tempCal);
            cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
            fclose(calendarFile);
            return INV_ALARM;
          }
          strcpy(calAlarm->action,stringPointer);
        }
        else if(strcmp(stringPointer,"TRIGGER") == 0)
        {
          stringPointer = strtok(NULL,"");

          if(stringPointer == NULL)
          {
            deleteCalendar(tempCal);
            cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
            fclose(calendarFile);
            return INV_ALARM;
          }
          strcpy(calAlarm->trigger,stringPointer);
        }
        else if (strcmp(contentLines[i],"END:VALARM") == 0)
        {
          if(strlen(calAlarm->action) == 0 || strlen(calAlarm->trigger)== 0)
          {
            deleteCalendar(tempCal);
            cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
            fclose(calendarFile);
            return INV_ALARM;
          }

          alarmFlag = false;
          insertBack(calEvent->alarms,calAlarm);
        }
        else
        {
          if( (isalpha(contentLines[i][0]) == 0 &&
            (contentLines[i][0] < '0' || contentLines[i][0] > '9') ) )
          {
            deleteCalendar(tempCal);
            cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
            fclose(calendarFile);
            return INV_ALARM;
          }
          alarmProperty = initializeProperty(contentLines[i]);
          strcpy(alarmProperty->propName,stringPointer);
          stringPointer = strtok(NULL,"");

          //missing description:
          if(stringPointer == NULL)
          {
            deleteCalendar(tempCal);
            cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
            fclose(calendarFile);
            return INV_ALARM;
          }
          strcpy(alarmProperty->propDescr,stringPointer);
          insertBack(calAlarm->properties,alarmProperty);
        }
      }
      else if (strcmp(contentLines[i],"END:VALARM") == 0)
      {
        //check if end alarm occured without ever beginning
        if(alarmFlag == false)
        {
          deleteCalendar(tempCal);
          cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
          fclose(calendarFile);
          return INV_ALARM;
        }
      }
      else if(strcmp(contentLines[i], "END:VEVENT") == 0)
      {
        //check to see if DTSTAMP missing
        if(strlen(calEvent->creationDateTime.date) == 0 || strlen(calEvent->creationDateTime.time) == 0)
        {
          deleteCalendar(tempCal);
          cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
          fclose(calendarFile);
          return INV_EVENT;
        }
        //check if the UID is not 0 and make sure an event began before closing
        else if( (strlen(calEvent->UID) == 0) || eventFlag == false)
        {
          deleteCalendar(tempCal);
          cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
          fclose(calendarFile);
          return INV_EVENT;
        }
        //the event was closed before an alarm was closed
        else if(alarmFlag == true)
        {
          deleteCalendar(tempCal);
          cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
          fclose(calendarFile);
          return INV_ALARM;
        }
        eventFlag = false;
        insertBack(tempCal->events,calEvent);
      }
      else
      {
        if( (isalpha(contentLines[i][0]) == 0 &&
          (contentLines[i][0] < '0' || contentLines[i][0] > '9') ) )
        {
          deleteCalendar(tempCal);
          cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
          fclose(calendarFile);
          return INV_EVENT;
        }
        eventProperty = initializeProperty(contentLines[i]);
        strcpy(eventProperty->propName,stringPointer);
        stringPointer = strtok(NULL,"");

        //missing description:
        if(stringPointer == NULL)
        {
          deleteCalendar(tempCal);
          cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
          fclose(calendarFile);
          return INV_EVENT;
        }
        strcpy(eventProperty->propDescr,stringPointer);
        insertBack(calEvent->properties,eventProperty);
      }
    }
    else if(strcmp(contentLines[i], "END:VEVENT") == 0)
    {
      if(eventFlag == false)
      {
        deleteCalendar(tempCal);
        cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
        fclose(calendarFile);
        return INV_EVENT;
      }
    }
    else
    {
      if( (isalpha(contentLines[i][0]) == 0 &&
        (contentLines[i][0] < '0' || contentLines[i][0] > '9') ) )
      {
        deleteCalendar(tempCal);
        cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
        fclose(calendarFile);
        return INV_CAL;
      }
      calProperty = initializeProperty(contentLines[i]);
      strcpy(calProperty->propName,stringPointer);
      stringPointer = strtok(NULL,"");

      //missing description:
      if(stringPointer == NULL)
      {
        deleteCalendar(tempCal);
        cleanUp(calEvent,calAlarm,calProperty,eventProperty,alarmProperty);
        fclose(calendarFile);
        return INV_CAL;
      }
      strcpy(calProperty->propDescr,stringPointer);
      insertBack(tempCal->properties,calProperty);
    }
    i++;
  }

  /***************************VALIDATION********************************/
  if(getFromFront(tempCal->events) == NULL)
  {
    deleteCalendar(tempCal);
    fclose(calendarFile);
    return INV_CAL;

  }
  //check for missing PRODID or VERSION
  if( (strlen(tempCal->prodID) == 0) || tempCal->version == 0.0)
  {
    deleteCalendar(tempCal);
    fclose(calendarFile);
    return INV_CAL;
  }
  //check if event closed
  if(eventFlag == true)
  {
    deleteCalendar(tempCal);
    fclose(calendarFile);
    return INV_EVENT;
  }
  //check if alarm closed
  if(alarmFlag == true)
  {
    deleteCalendar(tempCal);
    fclose(calendarFile);
    return INV_ALARM;
  }
  if(getLength(tempCal->events) == 0)
  {
    deleteCalendar(tempCal);
    fclose(calendarFile);
    return INV_CAL;
  }

  *obj = tempCal;
  fclose(calendarFile);
  return OK;
}

 char* printCalendar(const Calendar* obj)
 {
   char* printableString = NULL;
   char tempBuffer[100] = "";

   printableString = malloc(100* sizeof(char));

   if(obj == NULL)
   {
     return NULL;
   }

   strcpy(printableString,"VERSION: ");
   snprintf(tempBuffer,sizeof(tempBuffer),"%.2f", obj->version);
   strcat(printableString,tempBuffer);
   strcat(printableString," PRODID: ");
   strcat(printableString,obj->prodID);

   return printableString;
 }

 void deleteCalendar(Calendar* obj)
 {
   ListIterator eventIter;
   ListIterator alarmIter;
   Event *tempEvent = NULL;
   Event *nextEvent = NULL;
   Alarm *tempAlarm = NULL;
   Alarm *nextAlarm = NULL;

   if(obj == NULL)
   {
     return;
   }

   eventIter = createIterator(obj->events);
   tempEvent = nextElement(&eventIter);

   while(tempEvent != NULL)
   {
     if(tempEvent->alarms != NULL)
     {
       alarmIter = createIterator(tempEvent->alarms);
       tempAlarm = nextElement(&alarmIter);

       while(tempAlarm != NULL)
       {
         nextAlarm = nextElement(&alarmIter);
         freeList(tempAlarm->properties);
         free(tempAlarm->trigger);
         tempAlarm = nextAlarm;
       }
       freeList(tempEvent->alarms);
     }
     nextEvent = nextElement(&eventIter);
     if(tempEvent->properties != NULL)
     {
       freeList(tempEvent->properties);
     }
     tempEvent = nextEvent;
   }
   //free the calendar object
   if(obj->events != NULL)
   {
     freeList(obj->events);
   }
   if(obj->properties != NULL)
   {
     freeList(obj->properties);
   }
   free(obj);
 }

 char* printError(ICalErrorCode err)
 {
   char* errorString = NULL;

   if(err < 0 || err > 11)
   {
     return NULL;
   }

   errorString = malloc(100* sizeof(char));

   switch (err)
   {
     case 0:
            strcpy(errorString,"There is no error! The calendar is OK!");
            break;
     case 1:
            strcpy(errorString,"Error: The calendar file is invalid!");
            break;
     case 2:
            strcpy(errorString,"Error: The calendar is in a invalid Format!");
            break;
     case 3:
            strcpy(errorString,"Error: There was something wrong with the calendar version!");
            break;
     case 4:
            strcpy(errorString,"Error: There were duplicate calendar versions!");
            break;
     case 5:
            strcpy(errorString,"Error: There was something wrong with the calendar prodID!");
            break;
     case 6:
            strcpy(errorString,"Error: There were duplicate calendar prodID!");
            break;
     case 7:
            strcpy(errorString,"Error: There was something wrong with an Event!");
            break;
     case 8:
            strcpy(errorString,"Error: There was something wrong with the DateTime!");
            break;
     case 9:
            strcpy(errorString,"Error: There was something wrong with an Alarm!");
            break;
     case 10:
            strcpy(errorString,"Error: There was a Write Error!");
            break;
     case 11:
            strcpy(errorString,"Error: Some other type of Error has occured!");
            break;
   }

   return errorString;
 }
/***************************************STUBS************************************************/
ICalErrorCode writeCalendar(char* fileName, const Calendar* obj)
{
  FILE *calendarFile = NULL;
  char *extensionName = NULL;
  Property *currentProperty = NULL;
  Property *currentEventProperty = NULL;
  Property *currentAlarmProperty = NULL;
  Alarm *currentAlarm = NULL;
  Event *currentEvent = NULL;
  ListIterator generalPropertyItr;
  ListIterator eventIter;
  ListIterator eventPropertyIter;
  ListIterator alarmIter;
  ListIterator alarmPropertyIter;

  //validate the calendar object
  if(obj == NULL)
  {
    return INV_CAL;
  }

  //check if the file name, extension are correct and if it exists
  extensionName = strrchr(fileName,'.');
  if(fileName == NULL || (strcmp(extensionName,".ics") != 0) || strlen(fileName) == 0)
  {
   obj = NULL;
   return INV_FILE;
  }

  calendarFile = fopen(fileName, "w");
  if(access(fileName, F_OK) == -1 || access(fileName, W_OK) == -1 )
  {
   obj = NULL;
   return INV_FILE;
  }

  //initial setup
  generalPropertyItr = createIterator(obj->properties);
  eventIter = createIterator(obj->events);

  //setup the beginning of the calendar file
  fprintf(calendarFile,"BEGIN:VCALENDAR\r\n");
  fprintf(calendarFile,"VERSION:%0.1f\r\n",obj->version);
  fprintf(calendarFile,"PRODID:%s\r\n",obj->prodID);

  //write the iCal general properties to file
  while(1)
  {
    currentProperty = nextElement(&generalPropertyItr);
    if(currentProperty == NULL)
    {
      break;
    }
    fprintf(calendarFile,"%s:%s\r\n",currentProperty->propName,currentProperty->propDescr);
  }

  //write the iCal events
  while(1)
  {
    currentEvent = nextElement(&eventIter);
    if(currentEvent == NULL)
    {
      break;
    }
    fprintf(calendarFile,"BEGIN:VEVENT\r\n");

    //write the event creation date and time
    fprintf(calendarFile,"DTSTAMP:%sT%s",currentEvent->creationDateTime.date,currentEvent->creationDateTime.time);
    if(currentEvent->creationDateTime.UTC == true)
    {
      fprintf(calendarFile,"Z\r\n");
    }
    else
    {
      fprintf(calendarFile,"\r\n");
    }
    //write the event start date and time
    fprintf(calendarFile,"DTSTART:%sT%s",currentEvent->startDateTime.date,currentEvent->startDateTime.time);
    if(currentEvent->startDateTime.UTC == true)
    {
      fprintf(calendarFile,"Z\r\n");
    }
    else
    {
      fprintf(calendarFile,"\r\n");
    }
    //write the event UID
    fprintf(calendarFile,"UID:%s\r\n",currentEvent->UID);

    //iterate through the event properties and write to file
    eventPropertyIter = createIterator(currentEvent->properties);
    while(1)
    {
      currentEventProperty = nextElement(&eventPropertyIter);
      if(currentEventProperty == NULL)
      {
        break;
      }
      fprintf(calendarFile,"%s:%s\r\n",currentEventProperty->propName,currentEventProperty->propDescr);
    }

    //iterate through the alarms that belong to the current event
    alarmIter = createIterator(currentEvent->alarms);
    while(1)
    {
      currentAlarm = nextElement(&alarmIter);
      if(currentAlarm == NULL)
      {
        break;
      }
      fprintf(calendarFile,"BEGIN:VALARM\r\n");
      fprintf(calendarFile,"ACTION:%s\r\n",currentAlarm->action);
      fprintf(calendarFile,"TRIGGER:%s\r\n",currentAlarm->trigger);

      //iterate through the property list of the current alarm
      alarmPropertyIter = createIterator(currentAlarm->properties);
      while(1)
      {
        currentAlarmProperty = nextElement(&alarmPropertyIter);
        if(currentAlarmProperty == NULL)
        {
          break;
        }
        fprintf(calendarFile,"%s:%s\r\n",currentAlarmProperty->propName,currentAlarmProperty->propDescr);
      }
      fprintf(calendarFile,"END:VALARM\r\n");
    }
    fprintf(calendarFile,"END:VEVENT\r\n");
  }
  fprintf(calendarFile,"END:VCALENDAR\r\n");

  fclose(calendarFile);
  return OK;
}
ICalErrorCode validateCalendar(const Calendar* obj)
{
  ListIterator eventIter;
  ListIterator eventPropertyIter;
  ListIterator compEventPropIter;
  ListIterator alarmIter;
  ListIterator alarmPropertyIter;
  ListIterator compAlarmPropIter;
  ListIterator calPropIter;
  ListIterator ComparePropIter;
  Event *currentEvent = NULL;
  Alarm *currentAlarm = NULL;
  Property *calProperty = NULL;
  Property *nextCalProperty = NULL;
  Property *currentEventProperty = NULL;
  Property *nextEventProperty = NULL;
  Property *currentAlarmProperty = NULL;
  Property *nextAlarmProperty = NULL;
  bool eventDurationFlag = false;
  bool dtendFlag = false;
  bool alarmDurationFlag = false;
  bool alarmRepeatFlag = false;

  if(obj == NULL)
  {
//printf("TEST0\n");
    return INV_CAL;
  }

  /*****************************Validate the iCal struct********************************************/

  //validate the ical struct
  if(obj->version == 0.0 || strlen(obj->prodID) > 1000 || strlen(obj->prodID) == 0)
  {
//printf("TEST1\n");
    return INV_CAL;
  }
  else if (obj->events == NULL || getFromFront(obj->events) == NULL)
  {
//printf("TEST2\n");
    return INV_CAL;
  }
  else if(obj->properties == NULL)
  {
//printf("TEST3\n");
    return INV_CAL;
  }

  //validate the calenders properties
  calPropIter = createIterator(obj->properties);
  while(1)
  {
    calProperty = nextElement(&calPropIter); //points to first Property

    if(calProperty == NULL)
    {
      break;
    }
    else if(strlen(calProperty->propName) > 199 || strlen(calProperty->propName) == 0 ||
            strlen(calProperty->propDescr) == 0 )
    {
//printf("TEST4\n");
      return INV_CAL;
    }
    else if(calProperty->propName[0] == 'X' && calProperty->propName[1] == '-')
    {
//printf("TEST5\n");
      return INV_CAL;

    }
    else if(strcmp(calProperty->propName,"REQUEST-STATUS") == 0)
    {
//printf("TEST6\n");
      return INV_CAL;
    }
    else if(strcmp(calProperty->propName,"CALSCALE") != 0 && strcmp(calProperty->propName,"METHOD") != 0 )
    {
//printf("TEST7\n");
      return INV_CAL;
    }

    //we need two iterators to go through the list and check for multiple occurences
    ComparePropIter = createIterator(obj->properties);
    while(1)
    {
      nextCalProperty = nextElement(&ComparePropIter); // points to the first Property, yet again

      if(nextCalProperty == NULL)
      {
        break;
      }

      if( strcmp(nextCalProperty->propName,calProperty->propName) == 0 &&
         strcmp(nextCalProperty->propDescr,calProperty->propDescr) )
      {
        break;
      }
    }
    //at this point both iterators should be seperately pointing to the same object
    while(1)
    {
      nextCalProperty = nextElement(&ComparePropIter); //points to the element after calPropIter

      if(nextCalProperty == NULL)
      {
        break;
      }

      if( strcmp(nextCalProperty->propName,calProperty->propName) == 0 &&
          strcmp(nextCalProperty->propDescr,calProperty->propDescr) == 0 )
      {
        //check to see if any of the MUST appear once properties occur more than once
        if(strcmp(calProperty->propName,"CALSCALE") == 0 || strcmp(calProperty->propName,"METHOD") == 0 )
        {
//printf("TEST8\n");
          return INV_CAL;
        }
        if(strcmp(calProperty->propName,"VERSION") == 0 || strcmp(calProperty->propName,"PRODID") == 0 )
        {
//printf("TEST9\n");
          return INV_CAL;
        }
      }
    }
  }
  /*****************************Validate the events of the Calendar********************************************/
  //create iterator to go through each event in calendar
  eventIter = createIterator(obj->events);
  while(1)
  {
    currentEvent = nextElement(&eventIter);

    if(currentEvent == NULL)
    {
      break;
    }
    //check the UID of the event
    else if( strlen(currentEvent->UID) > 999 || strlen(currentEvent->UID) == 0 )
    {
      return INV_EVENT;
    }
    //check the date-time of the event (this should have been done in createCalendar)
    else if( (strlen(currentEvent->creationDateTime.date) < 8) || (strlen(currentEvent->creationDateTime.date) == 0) ||
             (strlen(currentEvent->creationDateTime.time) < 6) || (strlen(currentEvent->creationDateTime.time) == 0) ||
             (strlen(currentEvent->startDateTime.date) < 8) || (strlen(currentEvent->startDateTime.date) == 0) ||
             (strlen(currentEvent->startDateTime.time) < 6) || (strlen(currentEvent->startDateTime.time) == 0) )
    {
      return INV_EVENT;
    }
    else if(currentEvent->properties == NULL || currentEvent->alarms == NULL)
    {
      return INV_EVENT;
    }

    /*****************************Validate the properties of the event ********************************************/
    eventPropertyIter = createIterator(currentEvent->properties);
    while(1)
    {
      currentEventProperty = nextElement(&eventPropertyIter); //points to the first event property

      if( currentEventProperty == NULL)
      {
        break;
      }
      else if(strlen(currentEventProperty->propName) > 199 || strlen(currentEventProperty->propName) == 0 ||
              strlen(currentEventProperty->propDescr) == 0 )
      {
        return INV_EVENT;
      }
      else if(currentEventProperty->propName[0] == 'X' && currentEventProperty->propName[1] == '-')
      {
        return INV_EVENT;
      }
      else if(strcmp(currentEventProperty->propName,"REQUEST-STATUS") == 0)
      {
        return INV_EVENT;
      }
      //ensure that DTEND and DURATION dont occur in the same event
      else if(strcmp(currentEventProperty->propName,"DTEND") == 0)
      {
        if(eventDurationFlag == true)
        {
          return INV_EVENT;
        }
        dtendFlag = true;
      }
      else if(strcmp(currentEventProperty->propName,"DURATION") == 0)
      {
        if(dtendFlag == true)
        {
          return INV_EVENT;
        }
        eventDurationFlag = true;
      }
      //check to see if any of the hardcoded properties in the event struct show up in properties
      else if(strcmp(currentEventProperty->propName,"DTSTAMP") == 0 || strcmp(currentEventProperty->propName,"UID") == 0 ||
              strcmp(currentEventProperty->propName,"DTSTART") == 0)
      {
        return INV_EVENT;
      }
      //check to see if only valid properties are present
      else if( strcmp(currentEventProperty->propName,"CLASS") != 0 && strcmp(currentEventProperty->propName,"CREATED") != 0 &&
               strcmp(currentEventProperty->propName,"DESCRIPTION") != 0 && strcmp(currentEventProperty->propName,"GEO") != 0 &&
               strcmp(currentEventProperty->propName,"LAST-MOD") != 0 && strcmp(currentEventProperty->propName,"LOCATION") != 0 &&
               strcmp(currentEventProperty->propName,"ORGANIZER") != 0 && strcmp(currentEventProperty->propName,"PRIORITY") != 0 &&
               strcmp(currentEventProperty->propName,"SEQ") != 0 && strcmp(currentEventProperty->propName,"STATUS") != 0 &&
               strcmp(currentEventProperty->propName,"SUMMARY") != 0 && strcmp(currentEventProperty->propName,"TRANSP") != 0 &&
               strcmp(currentEventProperty->propName,"URL") != 0 && strcmp(currentEventProperty->propName,"RECURID") != 0 &&
               strcmp(currentEventProperty->propName,"ATTACH") != 0 && strcmp(currentEventProperty->propName,"ATTENDEE") != 0 &&
               strcmp(currentEventProperty->propName,"CATEGORIES") != 0 && strcmp(currentEventProperty->propName,"COMMENT") != 0 &&
               strcmp(currentEventProperty->propName,"CONTACT") != 0 && strcmp(currentEventProperty->propName,"EXDATE") != 0 &&
               strcmp(currentEventProperty->propName,"RRULE") != 0 && strcmp(currentEventProperty->propName,"RELATED") != 0 &&
               strcmp(currentEventProperty->propName,"RESOURCES") != 0 && strcmp(currentEventProperty->propName,"RDATE") != 0 )
      {
        return INV_EVENT;
      }

      //go through the list of event properties and check for multiple occurences and other conformance via 2 iters
      compEventPropIter = createIterator(currentEvent->properties);
      while(1)
      {
        nextEventProperty = nextElement(&compEventPropIter); //points to the first event property, yet again

        if(nextEventProperty == NULL)
        {
          break;
        }
        //make the iterators point to the same object in the list
        if(strcmp(currentEventProperty->propName,nextEventProperty->propName) == 0 &&
           strcmp(currentEventProperty->propDescr,nextEventProperty->propDescr) == 0 )
        {
          break;
        }
      }

      while(1)
      {
        nextEventProperty = nextElement(&compEventPropIter); // points to first element after eventPropertyIter

        if(nextEventProperty == NULL)
        {
          break;
        }
        //if a match is found, and if its one of the properties that can't occur more than once, return error code
        if(strcmp(currentEventProperty->propName,nextEventProperty->propName) == 0)
        {
          if(strcmp(currentEventProperty->propName,"CLASS") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"CREATED") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"DESCRIPTION") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"GEO") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"LAST-MOD") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"LOCATION") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"ORGANIZER") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"PRIORITY") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"SEQ") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"STATUS") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"SUMMARY") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"TRANSP") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"URL") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"RECURID") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"DTEND") == 0)
          {
            return INV_EVENT;
          }
          else if (strcmp(currentEventProperty->propName,"DURATION") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"DTSTAMP") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"UID") == 0)
          {
            return INV_EVENT;
          }
          else if(strcmp(currentEventProperty->propName,"DTSTART") == 0)
          {
            return INV_EVENT;
          }
        }
      }
    }
    //reset the flags for the next event
    dtendFlag = false;
    eventDurationFlag = false;

    /*****************************Validate the alarms of the event********************************************/
    alarmIter = createIterator(currentEvent->alarms);
    while(1)
    {
      currentAlarm = nextElement(&alarmIter);

      if(currentAlarm == NULL)
      {
        break;
      }
      //validate the alarm struct
      else if(strlen(currentAlarm->action) > 199 || strlen(currentAlarm->action) == 0)
      {
        return INV_ALARM;
      }
      else if (currentAlarm->trigger == NULL || strlen(currentAlarm->trigger) == 0 )
      {
        return INV_ALARM;
      }
      else if(currentAlarm->properties == NULL)
      {
        return INV_ALARM;
      }
      //check what type of Alarm you currently have
      else if(strcmp(currentAlarm->action,"AUDIO") != 0)
      {
        return INV_ALARM;
      }

      /***************************Validate the properties of the alarm for each event ****************************/
      alarmPropertyIter = createIterator(currentAlarm->properties);
      while(1)
      {
        currentAlarmProperty = nextElement(&alarmPropertyIter);

        if(currentAlarmProperty == NULL)
        {
          break;
        }
        else if(strlen(currentAlarmProperty->propName) > 199 || strlen(currentAlarmProperty->propName) == 0 ||
                strlen(currentAlarmProperty->propDescr) == 0 )
        {
          return INV_ALARM;
        }
        else if(currentAlarmProperty->propName[0] == 'X' && currentAlarmProperty->propName[1] == '-')
        {
          return INV_ALARM;
        }
        else if(strcmp(currentAlarmProperty->propName,"REQUEST-STATUS") == 0)
        {
          return INV_ALARM;
        }
        //set the flags for the required properties
        else if(strcmp(currentAlarmProperty->propName,"DURATION") == 0)
        {
          alarmDurationFlag = true;
        }
        else if(strcmp(currentAlarmProperty->propName,"REPEAT") == 0)
        {
          alarmRepeatFlag = true;
        }

        //check to see that there is no invalid property in the property list for alarms
        else if( strcmp(currentAlarmProperty->propName,"DURATION") != 0 && strcmp(currentAlarmProperty->propName,"REPEAT") != 0 &&
                 strcmp(currentAlarmProperty->propName,"ATTACH") != 0 )
        {
          return INV_ALARM;
        }
        //go through the list of alarm properties and check for multiple occurences and other conformance via 2 iters
        compAlarmPropIter = createIterator(currentAlarm->properties);
        while(1)
        {
          nextAlarmProperty = nextElement(&compAlarmPropIter); //points to the first alarm property, yet again

          if(nextAlarmProperty == NULL)
          {
            break;
          }
          //this will ensure that the iterators point to the same object in the property list
          else if(strcmp(currentAlarmProperty->propName,nextAlarmProperty->propName) == 0 &&
                  strcmp(currentAlarmProperty->propDescr,nextAlarmProperty->propDescr) == 0)
          {
            break;
          }
        }

        while(1)
        {
          nextAlarmProperty = nextElement(&compAlarmPropIter); // points to one element after alarmPropertyIter

          if(nextAlarmProperty == NULL)
          {
            break;
          }
          //if this executes we have a duplicate and must return error
          else if(strcmp(currentAlarmProperty->propName,nextAlarmProperty->propName) == 0)
          {
            if(strcmp(currentAlarmProperty->propName,"DURATION") == 0 )
            {
              return INV_ALARM;
            }
            else if(strcmp(currentAlarmProperty->propName,"REPEAT") == 0)
            {
              return INV_ALARM;
            }
            else if(strcmp(currentAlarmProperty->propName,"ATTACH") == 0)
            {
              return INV_ALARM;
            }
            else if(strcmp(currentAlarmProperty->propName,"ACTION") == 0)
            {
              return INV_ALARM;
            }
            else if(strcmp(currentAlarmProperty->propName,"TRIGGER") == 0)
            {
              return INV_ALARM;
            }
          }
        }
      }
      //ensure that the duration and repeat alarm properties do not occur without one another
      if( (alarmDurationFlag == true && alarmRepeatFlag == false) ||
          (alarmDurationFlag == false && alarmRepeatFlag == true) )
      {
        return INV_ALARM;
      }

      //reset the alarm flags for the next alarm
      alarmDurationFlag = false;
      alarmRepeatFlag = false;
    }
  }
  return OK;
}

/************************************************JSON FUNCTIONS******************************************************/

char* dtToJSON(DateTime prop)
{
  char *jsonDT = NULL;

  jsonDT = calloc(1000, sizeof(char));

  if(prop.UTC == true)
  {
    snprintf(jsonDT,999,"{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":true}",prop.date,prop.time);
  }
  else
  {
    snprintf(jsonDT,999,"{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":false}",prop.date,prop.time);
  }

  return jsonDT;
}

char* eventToJSON(const Event* event)
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

  jsonDT = dtToJSON(event->startDateTime);
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
    snprintf(jsonEvent,9999,"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"\"}",
            jsonDT,numOfProps,numOfAlarms);
  }
  else
  {
    snprintf(jsonEvent,9999,"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\"}",
            jsonDT,numOfProps,numOfAlarms,eventProperty->propDescr);
  }

  free(jsonDT);
  return jsonEvent;
}

char* eventListToJSON(const List* eventList)
{
  char* jsonEventList = NULL;
  char* jsonEvent = NULL;
  ListIterator eventIter;
  Event *currentEvent = NULL;
  int eventListLength = 0;
  int eventCount = 1;

  jsonEventList = calloc(10000, sizeof(char));

  if(eventList == NULL)
  {
    snprintf(jsonEventList,9999,"[]");
    return jsonEventList;
  }

  eventListLength = getLength((List*)eventList);
  strcat(jsonEventList,"[");

  eventIter = createIterator((List*)eventList);
  while(1)
  {
    currentEvent = nextElement(&eventIter);

    if(currentEvent == NULL)
    {
      break;
    }

    jsonEvent = eventToJSON(currentEvent);
    strcat(jsonEventList,jsonEvent);

    if(eventCount < eventListLength)
    {
      strcat(jsonEventList,",");
    }
    eventCount++;
    free(jsonEvent);
  }
  strcat(jsonEventList,"]");


  return jsonEventList;
}

char* calendarToJSON(const Calendar* cal)
{
  char *jsonCalendar = NULL;
  int numOfProps = 0;
  int numOfEvents = 0;

  jsonCalendar = calloc(10000,sizeof(char));

  if(cal == NULL)
  {
    snprintf(jsonCalendar,9999,"{}");
    return jsonCalendar;
  }

  numOfProps = 2 + getLength(cal->properties);
  numOfEvents = getLength(cal->events);

  if(numOfEvents == 0)
  {
    snprintf(jsonCalendar,9999,"{}");
    return jsonCalendar;
  }

  snprintf(jsonCalendar,9999,"{\"version\":%d,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}",
           (int)cal->version,cal->prodID,numOfProps,numOfEvents);

  return jsonCalendar;
}

Calendar* JSONtoCalendar(const char* str)
{
  Calendar *theCalendar = NULL;
  char stringBuffer[500] = {0};
  char *tokenPtr = NULL;
  char delimVal[6] = "{}:,\"";

  if(str == NULL)
  {
    return NULL;
  }

  strcpy(stringBuffer,str);

  //initialize the Calendar
  theCalendar = malloc(sizeof(Calendar));
  theCalendar->version = 0.0;
  memset(theCalendar->prodID,'\0',sizeof(theCalendar->prodID));
  theCalendar->events = initializeList(printEvent,deleteEvent,compareEvents);
  theCalendar->properties = initializeList(printProperty, deleteProperty, compareProperties);

  //parse the stringBuffer
  tokenPtr = strtok(stringBuffer,delimVal);
  tokenPtr = strtok(NULL,delimVal);
  theCalendar->version = atof(tokenPtr);
  tokenPtr = strtok(NULL,delimVal);
  tokenPtr = strtok(NULL,delimVal);
  strcpy(theCalendar->prodID,tokenPtr);

  if(theCalendar->events == NULL || theCalendar->properties == NULL ||
     theCalendar->version == 0.0 || strlen(theCalendar->prodID) == 0)
  {
    return NULL;
  }

  return theCalendar;
}

Event* JSONtoEvent(const char* str)
{
  char stringBuffer[500] = {0};
  Event *theEvent = NULL;
  char *tokenPtr = NULL;
  char delimVal[6] = "{}:,\"";

  theEvent = initializeEvent();

  if(theEvent == NULL)
  {
    return NULL;
  }

  strcpy(stringBuffer,str);
  tokenPtr = strtok(stringBuffer,delimVal);
  tokenPtr = strtok(NULL,delimVal);
  strcpy(theEvent->UID,tokenPtr);

  if(theEvent->properties == NULL || theEvent->alarms == NULL || strlen(theEvent->UID) == 0)
  {
    return NULL;
  }

  return theEvent;
}

void addEvent(Calendar* cal, Event* toBeAdded)
{
  if(cal != NULL && toBeAdded != NULL)
  {
    insertBack(cal->events,toBeAdded);
  }
}
