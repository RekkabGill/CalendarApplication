'use strict'

//connecting to the database
const mysql = require('mysql');
var connection;

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ********************

//respond to GET list of files from uploads director
app.get('/uploads/', function(req , res)
{
  fs.readdir('./uploads/',(err,icsFiles) => {

    let uploadArray = [];
    let index = 0;
    for(var singleFile in icsFiles)
    {
      let fileExt = icsFiles[singleFile].split('.')[1];
      if(fileExt == "ics")
      {
        uploadArray[index] = icsFiles[singleFile];
        index++;
      }
    }
    res.send(uploadArray);
  });
});

//c functions shared library setup
let calendarParser = ffi.Library('./libcal', {
  'createJSONCal': ['string',['string']],
  'getEventList':['string',['string']],
  'makeCalFile': ['string',['string','string','string','string','string','string','string','string','string','string']],
  'addEventToCal': ['string',['string','string','string','string','string','string','string','string','string']],
  'getAlarmList': ['string',['string','string']],
  'getAlarmListForDB': ['string',['string','string']],
  'getPropList': ['string',['string','string']],
  'getPropListForDB': ['string',['string','string']]

});

app.get('/fileLog', function(req,res){

  fs.readdir('./uploads/',(err,icsFiles) => {

    let uploadArray = [];
    let index = 0;
    for(var singleFile in icsFiles)
    {
      let fileExt = icsFiles[singleFile].split('.')[1];
      if(fileExt == "ics")
      {
        uploadArray[index] = icsFiles[singleFile];
        index++;
      }
    }

    let fileInfo = [];
    let i = 0;
    for(var calFile in uploadArray)
    {
      let calJSON = calendarParser.createJSONCal("./uploads/" + uploadArray[calFile]);
      fileInfo[i] = JSON.parse(calJSON);
      fileInfo[i].name = uploadArray[calFile];
      i++;
    }
    res.send(fileInfo);
  });
});

//respond to get request of the calendar view panel to show event list
app.get('/calendarView', function(req,res){

  let eventJSON = calendarParser.getEventList("./uploads/" + req.query.theFileName);
  res.send(eventJSON);
});

//respond to get request to create a new calendar file
app.get('/createCalFile', function(req,res){

  let totalFileName = "./uploads/" + req.query.fileNameObj.theFileName


  let resultMessage = calendarParser.makeCalFile(

    totalFileName,
    JSON.stringify(req.query.calInfoObj),
    JSON.stringify(req.query.initialEventObj),
    req.query.eventInfo.initCD,
    req.query.eventInfo.initCT,
    req.query.eventInfo.createUTC,
    req.query.eventInfo.initSD,
    req.query.eventInfo.initST,
    req.query.eventInfo.startUTC,
    req.query.eventInfo.initSum
  );

  res.send(JSON.stringify(resultMessage));

});

//respond to the get request to add a new event to a calendar
app.get('/addEvent', function(req,res){

  let addEventFileName = "./uploads/" + req.query.eventCalName.theFileName

  let returnMessage = calendarParser.addEventToCal(

    addEventFileName,
    JSON.stringify(req.query.addEventObj),
    req.query.eventInfo.addEventCD,
    req.query.eventInfo.addEventCT,
    req.query.eventInfo.addCreateUTC,
    req.query.eventInfo.addEventSD,
    req.query.eventInfo.addEventST,
    req.query.eventInfo.addStartUTC,
    req.query.eventInfo.addSum
  );

  res.send(JSON.stringify(returnMessage));

});

//respond to show alarms request
app.get('/alarmList', function(req,res){

  let alarmJSON = calendarParser.getAlarmList("./uploads/" + req.query.theFileName, JSON.stringify(req.query.theEvent));
  res.send(alarmJSON);

});

//respond to show event optional properties request
app.get('/propList', function(req,res){

  let propJSON = calendarParser.getPropList("./uploads/" + req.query.theFileName, JSON.stringify(req.query.theEvent));
  res.send(propJSON);

});

/*****************************************DATABASE FEATURES *******************************************************************/

/***************************LOGIN *************************/
app.get('/loginDatabase', function(req,res){

    let dbMsg = "";
    connection = mysql.createConnection({

    host : 'dursley.socs.uoguelph.ca',
    user : req.query.dbUser,
    password : req.query.dbPass,
    database : req.query.dbUser
  });

    connection.connect(function (error){
    if(error)
    {
      return res.status(500).send({error: "Failed to connect to database. Please Try Again"});
    }
    else
    {
      let errorFlag = 0;
      let dbTables = [

        "CREATE TABLE FILE (cal_id INT AUTO_INCREMENT PRIMARY KEY, file_Name VARCHAR(60) NOT NULL, version INT NOT NULL, prod_id VARCHAR(256) NOT NULL)",
        "CREATE TABLE EVENT (event_id INT AUTO_INCREMENT PRIMARY KEY, summary VARCHAR(1024), start_time DATETIME NOT NULL, location VARCHAR(60), organizer VARCHAR(256), cal_file INT NOT NULL, FOREIGN KEY(cal_file) REFERENCES FILE(cal_id) ON DELETE CASCADE)",
        "CREATE TABLE ALARM (alarm_id INT AUTO_INCREMENT PRIMARY KEY, action VARCHAR(256) NOT NULL, `trigger` VARCHAR(256) NOT NULL, event INT NOT NULL, FOREIGN KEY(event) REFERENCES EVENT(event_id) ON DELETE CASCADE)"
      ];

      connection.query(dbTables[0],function (err, rows, fields)
      {
        if (err)
        {
          console.log(err.sqlMessage);
        }

      });

      connection.query(dbTables[1],function (err, rows, fields)
      {
        if (err)
        {
          console.log(err.sqlMessage);
        }

      });

      connection.query(dbTables[2],function (err, rows, fields)
      {
        if (err)
        {
          console.log(err.sqlMessage);
        }

      });

      res.send({success: "connection established!"});
    }

  });
});

/*****************************************STORE ALL FILES IN TABLES****************************************************/

app.get('/dbStorage', function(req,res){

  fs.readdir('./uploads/',(err,icsFiles) => {

    var totalFiles = 0;
    var totalEvents = 0;
    var totalAlarms = 0;
    var dbStorageError = null;
    let uploadArray = [];
    let index = 0;
    for(var singularFile in icsFiles)
    {
      let fileExt = icsFiles[singularFile].split('.')[1];
      if(fileExt == "ics")
      {
        uploadArray[index] = icsFiles[singularFile];
        index++;
      }
    }

    let fileInfo = [];
    let i = 0;
    for(var theCalFile in uploadArray)
    {
      let calJSON = calendarParser.createJSONCal("./uploads/" + uploadArray[theCalFile]);
      fileInfo[i] = JSON.parse(calJSON);
      fileInfo[i].name = uploadArray[theCalFile];
      i++;
    }

/*
    //TESTING STATION:
    let eventListJSON = calendarParser.getEventList("./uploads/" + fileInfo[2].name);
    let eventStrings = JSON.parse(eventListJSON);
    let eventDateString = "STR_TO_DATE('" + eventStrings[0].startDT.date + "," + eventStrings[0].startDT.time + "','" + "%Y%m%d,%H%i%s')";
    console.log(eventDateString);
    let dbPropJSON = calendarParser.getPropListForDB("./uploads/" + fileInfo[2].name, JSON.stringify(eventStrings[0]));
    let parsedProperty = JSON.parse(dbPropJSON);

    let alarmJSON = calendarParser.getAlarmListForDB("./uploads/" + fileInfo[2].name, JSON.stringify(eventStrings[0]));
    let parsedAlarm = JSON.parse(alarmJSON);
  //  console.log(parsedAlarm[0]);
*/

    function dbStorageCallBack()
    {
      if(dbStorageError == null)
      {
        res.send(["Database has N" + totalFiles + " Files, N" + totalEvents + " events, and N" + totalAlarms +" alarms"]);
      }
      else
      {
        res.status(500).send({error: "Failed to store all files in the database"});
      }
    }


    //clear the tables before storing new ones
    connection.query("DELETE FROM FILE", function (err,rows,fields)
    {
      if(err)
      {
        console.log(err.sqlMessage);
      }
    });
    //return the auto increment back to 1 for the FILE table
    connection.query("ALTER TABLE FILE AUTO_INCREMENT = 1", function (err,rows,fields)
    {
      if(err)
      {
        console.log(err.sqlMessage);
      }
    });
    //clear the EVENT table before storing new ones
    connection.query("DELETE FROM EVENT", function (err,rows,fields)
    {
      if(err)
      {
        console.log(err.sqlMessage);
      }
    });
    //return the auto increment back to 1 for the EVENT table
    connection.query("ALTER TABLE EVENT AUTO_INCREMENT = 1", function (err,rows,fields)
    {
      if(err)
      {
        console.log(err.sqlMessage);
      }
    });
    //clear the ALARM table before storing new ones
    connection.query("DELETE FROM ALARM", function (err,rows,fields)
    {
      if(err)
      {
        console.log(err.sqlMessage);
      }
    });
    //return the auto increment back to 1 for the ALARM table
    connection.query("ALTER TABLE ALARM AUTO_INCREMENT = 1", function (err,rows,fields)
    {
      if(err)
      {
        console.log(err.sqlMessage);
      }
    });

    let eventCount = 1;
    //fill in the tables with all the new information
    for( var calFileInfo in fileInfo)
    {
      let insertString = "INSERT INTO FILE (file_Name, version, prod_id) VALUES ('" + fileInfo[calFileInfo].name + "','" + fileInfo[calFileInfo].version + "','" + fileInfo[calFileInfo].prodID + "')";

      connection.query(insertString, function (err,rows,fields)
      {
        if(err)
        {
          dbStorageError = err.sqlMessage;
          console.log(err.sqlMessage);
        }
        else
        {
          totalFiles++;
        }
      });

      //add all the events to the EVENT TABLE that are in the current calendar File
      let eventListJSON = calendarParser.getEventList("./uploads/" + fileInfo[calFileInfo].name);
      let eventStrings = JSON.parse(eventListJSON);

      for(let singleEvent in eventStrings)
      {
        let dbPropJSON = calendarParser.getPropListForDB("./uploads/" + fileInfo[calFileInfo].name, JSON.stringify(eventStrings[singleEvent]));
        let eventLocation = null;
        let eventOrganizer = null;

        if(dbPropJSON != null)
        {
          let parsedProperties = JSON.parse(dbPropJSON);

          for(let parsedProperty in parsedProperties)
          {
            if(parsedProperties[parsedProperty].propName == 'ORGANIZER')
            {
              eventOrganizer = parsedProperties[parsedProperty].propDescr;
            }
            else if(parsedProperties[parsedProperty].propName == 'LOCATION')
            {
              eventLocation = parsedProperties[parsedProperty].propDescr;
            }
          }
        }
        let eventDateString = "SELECT STR_TO_DATE('" + eventStrings[singleEvent].startDT.date + "," + eventStrings[singleEvent].startDT.time + "','" + "%Y%m%d,%H%i%s')";
        let insertEventString = "INSERT INTO EVENT(summary, start_time, location, organizer, cal_file) VALUES ('" + eventStrings[singleEvent].summary +"',(" + eventDateString + "),'" + eventLocation + "','" + eventOrganizer + "'," + "(SELECT cal_id FROM FILE WHERE file_Name ='" + fileInfo[calFileInfo].name +"'))";
        connection.query(insertEventString, function(err,rows,fields)
        {
          if(err)
          {
            dbStorageError = err.sqlMessage;
            console.log(err.sqlMessage);
          }
          else
          {
            totalEvents++;
          }
        });

        //add all the alarms into the alarm table
        let alarmJSON = calendarParser.getAlarmListForDB("./uploads/" + fileInfo[calFileInfo].name, JSON.stringify(eventStrings[singleEvent]));

        if(alarmJSON != null)
        {
          let parsedAlarms = JSON.parse(alarmJSON);

          for(let someAlarm in parsedAlarms)
          {

            //let insertAlarmString = "INSERT INTO ALARM(action, `trigger`, event) VALUES ('" + parsedAlarms[someAlarm].action + "','" + parsedAlarms[someAlarm].trigger + "'," + "(SELECT event_id from EVENT WHERE (summary ='" + eventStrings[singleEvent].summary + "') AND (start_time = (" + eventDateString +")) AND (organizer ='" + eventOrganizer + "')))";
            let insertAlarmString = "INSERT INTO ALARM(action, `trigger`, event) VALUES ('" + parsedAlarms[someAlarm].action + "','" + parsedAlarms[someAlarm].trigger + "'," + "(SELECT event_id from EVENT WHERE event_id = " + eventCount + "))";

            connection.query(insertAlarmString, function(err,rows,fields)
            {
              if(err)
              {
                dbStorageError = err.sqlMessage;
                console.log(err.sqlMessage);
              }
            });
            totalAlarms++;
          }
        }
        eventCount++;
        totalEvents++;
      }
      totalFiles++;
    }
    dbStorageCallBack();

  }); //readdir bracket

}); //dbStorage bracket

/******************************************CLEAR ALL DATA IN TABLES *****************************************************/
app.get('/dbClear', function(req,res){

  var dbClearError = null;

  function dbClearCallBack()
  {
    if(dbClearError == null)
    {
      res.send({success: "The database was cleared successfully"});
    }
    else
    {
      res.status(500).send({error: "Failed to clear the database."});
    }
  }
  //clear the tables before storing new ones
  connection.query("DELETE FROM FILE", function (err,rows,fields)
  {
    if(err)
    {
      console.log(err.sqlMessage);
      dbClearError = err.sqlMessage;
    }
  });

  //return the auto increment back to 1 for the FILE table
  connection.query("ALTER TABLE FILE AUTO_INCREMENT = 1", function (err,rows,fields)
  {
    if(err)
    {
      console.log(err.sqlMessage);
      dbClearError = err.sqlMessage;
    }
  });
  //clear the EVENT table before storing new ones
  connection.query("DELETE FROM EVENT", function (err,rows,fields)
  {
    if(err)
    {
      console.log(err.sqlMessage);
      dbClearError = err.sqlMessage;
    }
  });
  //return the auto increment back to 1 for the EVENT table
  connection.query("ALTER TABLE EVENT AUTO_INCREMENT = 1", function (err,rows,fields)
  {
    if(err)
    {
      console.log(err.sqlMessage);
      dbClearError = err.sqlMessage;
    }
  });
  //clear the ALARM table before storing new ones
  connection.query("DELETE FROM ALARM", function (err,rows,fields)
  {
    if(err)
    {
      console.log(err.sqlMessage);
      dbClearError = err.sqlMessage;
    }
  });
  //return the auto increment back to 1 for the ALARM table
  connection.query("ALTER TABLE ALARM AUTO_INCREMENT = 1", function (err,rows,fields)
  {
    if(err)
    {
      console.log(err.sqlMessage);
      dbClearError = err.sqlMessage;
    }
    dbClearCallBack();
  });

});

/******************************************DISPLAY DB*****************************************************/


app.get('/dbDisplay', function(req,res){

  var filecCnt;
  var eventCnt;
  var alarmCnt;
  var dbDisplayError = null;

  function callBack()
  {
    if(dbDisplayError == null)
    {
      let returnObj = ["Database has N" + filecCnt + " Files, N" + eventCnt + " events, and N" + alarmCnt +" alarms"];
      res.send(returnObj);
    }
    else
    {
      res.status(500).send({error: "Failed to retrieve database tables count"});
    }
  }

  connection.query("SELECT COUNT (*) AS cntFile FROM FILE", function (err,result,fields)
  {
    if(err)
    {
      console.log(err.sqlMessage);
      dbDisplayError = err.sqlMessage;
    }
    else
    {
      filecCnt =  result[0].cntFile;
    }
  });

  connection.query("SELECT COUNT (*) AS cntEvent FROM EVENT", function (err,result,fields)
  {
    if(err)
    {
      console.log(err.sqlMessage);
      dbDisplayError = err.sqlMessage;
    }
    else
    {
      eventCnt =  result[0].cntEvent;
    }
  });

  connection.query("SELECT COUNT (*) AS cntAlarm FROM ALARM", function (err,result,fields)
  {
    if(err)
    {
      console.log(err.sqlMessage);
      dbDisplayError = err.sqlMessage;
      callBack();
    }
    else
    {
      //result[0].cntFile;
      alarmCnt =  result[0].cntAlarm;
      callBack();
    }
  });


});


/******************************************DISPLAY DB*****************************************************/

app.get('/showEventsDB', function(req,res){

  connection.query("SELECT * FROM EVENT ORDER BY start_time", function (err,rows,fields)
  {
    if(err)
    {
      console.log(err.sqlMessage);
      res.status(500).send({error: err.sqlMessage});
      return;

    }
    else
    {
      res.send(rows);
      return;
    }
  });

});

app.get('/showPartEventsDB', function(req,res){

  connection.query("SELECT cal_id FROM FILE WHERE file_Name ='" + req.query.fileName + "'", function (err,rows,fields)
  {
    if(err || rows.length == 0)
    {
      res.status(500).send({error: "failed to display events from file from database, check file name and file extension"});
      return;

    }
    else
    {
      let calID = rows[0].cal_id;
      connection.query("SELECT * FROM EVENT WHERE cal_file = " + calID , function (err,rows,fields)
      {
        if(err)
        {
          console.log(err.sqlMessage);
          res.status(500).send({error: "failed to display events from file from database, check file name and file extension"});
          return;

        }
        else
        {
          res.send(rows);
          return;
        }
      });
    }
  });

});













































app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
