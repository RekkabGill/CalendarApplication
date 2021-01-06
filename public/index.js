// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {

/**********************STATUS PANEL FUNCTIONS***********/
  function changeStatusPanel(string)
  {
    let table1 = 0;
    let row1 = 0;
    let statusCell = 0;

     table1 = document.getElementById("StatusPanel");
     row1 = table1.insertRow(row1.length + 1);
     statusCell = row1.insertCell(0);
     statusCell.innerHTML = string;
  }

  $("#clearButton").on("click",function(event)
  {
    let table1 = document.getElementById("StatusPanel");
    while(table1.rows.length > 0)
    {
      table1.deleteRow(0);
    }
  });

  /*******LIST ALL THE FILES IN THE UPLOADS DIRECTORY***********/

  var listObject = document.getElementById("selectFileList");
  listObject.addEventListener("click",changeCalendarView); //add an event listener to the list of files

  function updateList()
  {
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything
        url: '/uploads/',   //The server endpoint we are connecting to
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string,
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            //We write the object to the console to show that the request was successful
            selectList = document.getElementById("selectFileList");
            lengthOfList = selectList.length;

            for(let i = 0; i < lengthOfList; i++)
            {
              selectList.options[i] = null;
            }

            for (i = 0; i < data.length; i++)
            {
              let listOption = new Option("text", "value");
              $(listOption).html(data[i]);
              $("#selectFileList").append(listOption);
            }

        },
        fail: function(error) {
            // Non-200 return, do something with error
            changeStatusPanel(error);
        }
    });
  }
  updateList();

    /*******************UPDATE THE CALENDAR PANEL ***************************************************/
  function changeCalendarView()
  {

    var theFile = $('#selectFileList').find(":selected").text();

    var fileObj = {
      theFileName: theFile
    };

    if(fileObj.theFileName == "")
    {
      changeStatusPanel("No events to display");
    }
    else
    {
      $.ajax({
          type: 'get',            //Request type
          dataType: 'json',       //Data type - we will use JSON for almost everything
          url: '/calendarView',
          data: fileObj,   //The server endpoint we are connecting to
          success: function (data) {

            if(data.length == 0)
            {
              changeStatusPanel("No Events to Show");
            }
            else
            {
              changeStatusPanel("The Events of: " + theFile + " are listed.");

              var table3 = document.getElementById("CalViewPanel");

              //clear the previous information
              while(table3.rows.length > 1)
              {
                table3.deleteRow(1);
              }

              var eventNum = 0;
              var calTableRow = 1;
              for(var i = 0; i < data.length; i++)
              {
                //setup the variables
                var startDTobject = data[i].startDT;
                var row3 = table3.insertRow(calTableRow);

                //always number the event first
                var eventCell = row3.insertCell(0);
                eventCell.innerHTML = ++eventNum;

                //insert the date time object
                j = 1;
                for(var property1 in startDTobject)
                {
                  var timeCell1 = row3.insertCell(j);
                  timeCell1.innerHTML = startDTobject[property1];
                  j++;
                }

                //setup button for alarms
                let newAlarmButton = document.createElement('input');
                newAlarmButton.type = "button";
                newAlarmButton.class = "alarmsButton";
                newAlarmButton.value = "Alarms"
                newAlarmButton.id = "alarmButtonNum" + calTableRow;
                newAlarmButton.text = data[i];
                newAlarmButton.addEventListener("click",alarmFunction);

                //setup button for properties
                let newPropButton = document.createElement('input');
                newPropButton.type = "button";
                newPropButton.className = "PropsButton";
                newPropButton.value = "Properties"
                newPropButton.id = "PropNum" + calTableRow;
                newPropButton.text = data[i];
                newPropButton.addEventListener("click",optionalPropFunction);


                //insert the rest of the properties
                var propCell = row3.insertCell(j);
                var propButCell = row3.insertCell(j+1);
                var alarmCell = row3.insertCell(j+2);
                var alarmButCell = row3.insertCell(j+3)
                var eventSummaryCell = row3.insertCell(j+4);

                propCell.innerHTML =  data[i].numProps;
                propButCell.appendChild(newPropButton);
                alarmCell.innerHTML = data[i].numAlarms;
                alarmButCell.appendChild(newAlarmButton);
                eventSummaryCell.innerHTML = data[i].summary;

                calTableRow++;
              }
            }
          },
          fail: function(error) {
              // Non-200 return, do something with error
              changeStatusPanel("failed to load the event list in Calendar View Panel");
          }
      });
    }
  }
  /***************************SHOW ALARMS***********************************/
  function alarmFunction(event)
  {
    let alarmListFile = $('#selectFileList').find(":selected").text();
    let alarmEvent = event.target.text;
    var alarmFileObj = {
      theFileName: alarmListFile,
      theEvent: alarmEvent
    };

    //Pass data to the Ajax call, so it gets passed to the
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
      data: alarmFileObj,
      url: '/alarmList',   //The server endpoint we are connecting to
      success: function (data) {

        let alarmNumber = 0;
        let alarmListString;

        if(data.length == 0)
        {
          changeStatusPanel("No Alarms were loaded for display");
        }
        else
        {
          changeStatusPanel("^^^ALARM LIST^^^");
          for(let singleAlarm in data)
          {
            alarmNumber++;
            alarmListString = "Alarm " + alarmNumber + ": Action: " + data[singleAlarm].action + ", Trigger: " + data[singleAlarm].trigger + ", Alarm Properties: " + data[singleAlarm].numProps;
            changeStatusPanel(alarmListString);
          }
        }
      },
      fail: function(error) {
          // Non-200 return, do something with error
          changeStatusPanel(error);
      }

    });
  }

  /******************************SHOW PROPERTIES****************************************/
  function optionalPropFunction(event)
  {
    let propListFile = $('#selectFileList').find(":selected").text();
    let propEvent = event.target.text;
    var propFileObj = {
      theFileName: propListFile,
      theEvent: propEvent
    };

    //Pass data to the Ajax call, so it gets passed to the
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
      data: propFileObj,
      url: '/propList',   //The server endpoint we are connecting to
      success: function (data) {

        let propNumber = 0;
        let propListString;

        if(data.length == 0)
        {
          changeStatusPanel("No Optional Properties were loaded for display");
        }
        else
        {
          changeStatusPanel("^^^Optional Properties LIST^^^");
          for(let singleProp in data)
          {
            propNumber++;
            propListString = "Event Property " + propNumber + ": Property Name: " + data[singleProp].propName + ", Property Description: " + data[singleProp].propDescr;
            changeStatusPanel(propListString);
          }
        }
      },
      fail: function(error) {
          // Non-200 return, do something with error
          changeStatusPanel(error);
      }

    });
  }


  /*****************HAVE THE FILE LOG PANEL SHOW ALL THE FILES***********/
  function getFileLogCal()
  {
    var table2 = document.getElementById("FileLogPanel");

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything
        url: '/fileLog',
        success: function (data) {

          //clear the previous information
          while(table2.rows.length > 1)
          {
            table2.deleteRow(1);
          }

          if(data.length == 0)
          {
            var emptyRow1 = table2.insertRow(1);
            emptyRow1.innerHTML = "No Files";
          }
          else
          {

            changeStatusPanel("files loaded");

            let i = 1;
            let j = 0;

            for(j = 0; j < data.length; j++)
            {
              var row2 = table2.insertRow(i);
              var fileNameCell = row2.insertCell(0);
              var versionCell = row2.insertCell(1);
              var prodIDCell = row2.insertCell(2);
              var numOfEventsCell = row2.insertCell(3);
              var numOfPropsCell = row2.insertCell(4);

              let link = "/uploads/" + data[j].name;

              fileNameCell.innerHTML = '<a href ="'+link+'">'+data[j].name+'</a>';
              versionCell.innerHTML = data[j].version;
              prodIDCell.innerHTML = data[j].prodID;
              numOfEventsCell.innerHTML = data[j].numEvents;
              numOfPropsCell.innerHTML = data[j].numProps;
              i++;
            }
          }
        },
        fail: function(error) {
            // Non-200 return, do something with error
            changeStatusPanel(error);
        }
    });
  }
  getFileLogCal();

/**************UPLOAD BUTTON*****************************/
    $("#uploadButton").on("click",function(event)
    {

      var formData = new FormData(this);

        //Pass data to the Ajax call, so it gets passed to the
        $.ajax({
          type: 'POST',            //Request type
          dataType: 'json',       //Data type - we will use JSON for almost everything
          data: formData,
          cache: false,
          contentType: false,
          processData: false,
          url: '/upload',   //The server endpoint we are connecting to
          success: function (data) {
              /*  Do something with returned object
                  Note that what we get is an object, not a string,
                  so we do not need to parse it on the server.
                  JavaScript really does handle JSONs seamlessly
              */
              updateList();
              changeStatusPanel("Upload Successful");

          },
          fail: function(error) {
              // Non-200 return, do something with error
              changeStatusPanel(error);
          }

        });
    });


/**************CREATE CALENDAR*****************************/
  $("#createCalForm").on("submit",function(event)
  {
    event.preventDefault();

    let fileName =
    {
      theFileName: $('#calFileName').val()
    };

    let calInfo =

    {
      version:  $('#calVersion').val(),
      prodID:$('#calProdID').val()
    };

    let initialEvent =
    {
      UID: $('#eventID').val()
    };

    let eventInformation =
    {
      initCD:   $('#initCD').val(),
      initCT:   $('#initCT').val(),
      createUTC:$('#createUTC').val(),
      initSD:   $('#initSD').val(),
      initST:   $('#initST').val(),
      startUTC: $('#startUTC').val(),
      initSum:  $('#initSummary').val(),
    };

    let formData =
    {
      fileNameObj: fileName,
      calInfoObj: calInfo,
      initialEventObj: initialEvent,
      eventInfo: eventInformation
    };

    if(eventInformation.initCT.length != 6 || eventInformation.initST.length != 6)
    {
      changeStatusPanel("Create Calendar Error: Check The Time(s)");
    }
    else if(eventInformation.initCD.length != 8 || eventInformation.initSD.length != 8)
    {
      changeStatusPanel("Create Calendar Error: Check The Date(s)");
    }
    else if( (eventInformation.createUTC != "true" && eventInformation.createUTC != "false") ||
            (eventInformation.startUTC != "true" && eventInformation.startUTC != "false") )
    {
      changeStatusPanel("Create Calendar Error: The Time UTC value must be true or false (lowercase)");
    }
    else
    {

      //Pass data to the Ajax call, so it gets passed to the
      $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything
        data: formData,
        url: '/createCalFile',   //The server endpoint we are connecting to
        success: function (data) {

            changeStatusPanel(data);
            updateList();
            getFileLogCal();

        },
        fail: function(error) {
            // Non-200 return, do something with error
            changeStatusPanel(error);
        }
      });
    }
  });

//********************************************ADD AN EVENT***************************************/
  $("#addEventForm").on("submit",function(event)
  {
    event.preventDefault();
    let addEventCalendar = $('#selectFileList').find(":selected").text();

    let eventFileName =
    {
      theFileName: addEventCalendar
    };

    let addEvent =
    {
      UID: $('#addEventUID').val()
    };

    let eventInformation =
    {
      addEventCD:   $('#addEventCD').val(),
      addEventCT:   $('#addEventCT').val(),
      addCreateUTC: $('#addCreateUTC').val(),
      addEventSD:   $('#addEventSD').val(),
      addEventST:   $('#addEventST').val(),
      addStartUTC:  $('#addStartUTC').val(),
      addSum:  $('#addSummary').val(),
    };

    let formData =
    {
      eventCalName: eventFileName,
      addEventObj: addEvent,
      eventInfo: eventInformation
    };

    if(eventInformation.addEventCT.length != 6 || eventInformation.addEventST.length != 6)
    {
      changeStatusPanel("Add Event Error: Check The Time(s)");
    }
    else if(eventInformation.addEventCD.length != 8 || eventInformation.addEventSD.length != 8)
    {
      changeStatusPanel("Add Event Error: Check The Date(s)");
    }
    else if( (eventInformation.addCreateUTC != "true" && eventInformation.addCreateUTC != "false") ||
            (eventInformation.addStartUTC != "true" && eventInformation.addStartUTC != "false") )
    {
      changeStatusPanel("Add Event Error: The Time UTC value must be true or false (lowercase)");
    }
    else
    {
      //Pass data to the Ajax call, so it gets passed to the
      $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything
        data: formData,
        url: '/addEvent',   //The server endpoint we are connecting to
        success: function (data) {

            changeCalendarView();
            changeStatusPanel(data);

        },
        fail: function(error) {
            // Non-200 return, do something with error
            changeStatusPanel(error);
        }
      });
    }
  });

/******************************************DATABASE INFO *****************************************************/

/******************************************LOGIN *****************************************************/


  $("#loginScreen").on("submit",function(event)
  {
    event.preventDefault();

    let dbInfo =
    {
      dbUser: $('#userName').val(),
      dbPass: $('#userPass').val(),
      dbName: $('#dbName').val()
    };

    //Pass data to the Ajax call, so it gets passed to the
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
      data: dbInfo,
      url: '/loginDatabase',   //The server endpoint we are connecting to
      success: function (data) {

        changeStatusPanel("Database login successful");
        $('div#storeAllFiles').removeClass("hidden");
        $('div#displayDBStatus').removeClass("hidden");

      },
      error: function(error) {
          // Non-200 return, do something with error
          let parsedError = error.responseJSON;
          changeStatusPanel(parsedError.error);
          alert("login failed, please try again");
      }
    });

  });

/******************************************STORE ALL FILES *****************************************************/
  $("#storeFiles").on("click",function(event)
  {
    let storageFiles = document.getElementById("selectFileList");

    if ($("#selectFileList").val() == null || $("#selectFileList").val() == "")
    {
      changeStatusPanel("Store Files Error: No files to store");
    }
    else
    {
      //Pass data to the Ajax call, so it gets passed to the
      $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything
      //  data: dbStorageInfo,
        url: '/dbStorage',   //The server endpoint we are connecting to
        success: function (data) {

          changeStatusPanel("Store All FIles Successful:" + data[0]);
          $('div#clearAllData').removeClass("hidden");

        },
        error: function(error) {
            // Non-200 return, do something with error
            let parsedError = error.responseJSON;
            changeStatusPanel(parsedError.error);
        }
      }); //ajax bracket

    } // else bracket

  });

/******************************************CLEAR ALL DATA*****************************************************/

  $("#clearDB").on("click",function(event)
  {
    //Pass data to the Ajax call, so it gets passed to the
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
    //  data: dbStorageInfo,
      url: '/dbClear',   //The server endpoint we are connecting to
      success: function (data) {

        changeStatusPanel("Clear Database: database successfully cleared");

      },
      error: function(error) {
          // Non-200 return, do something with error
          let parsedError = error.responseJSON;
          changeStatusPanel(parsedError.error);
      }
    }); //ajax bracket

  });

  /******************************************DISPLAY DB*****************************************************/
  $("#displayDB").on("click",function(event)
  {
    //Pass data to the Ajax call, so it gets passed to the
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
    //  data: dbStorageInfo,
      url: '/dbDisplay',   //The server endpoint we are connecting to
      success: function (data) {

        changeStatusPanel(data[0]);

      },
      error: function(error) {
          // Non-200 return, do something with error
          let parsedError = error.responseJSON;
          changeStatusPanel(parsedError.error);
      }
    }); //ajax bracket

  });

/******************************************DISPLAY EVENTS*****************************************************/

  $("#ExecuteQuery1").on("click",function(event)
  {
    //Pass data to the Ajax call, so it gets passed to the
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
    //  data: dbStorageInfo,
      url: '/showEventsDB',   //The server endpoint we are connecting to
      success: function (data) {

        displayEventsQuery(data);


      },
      error: function(error) {
          // Non-200 return, do something with error
          let parsedError = error.responseJSON;
          changeStatusPanel(parsedError.error);
      }
    }); //ajax bracket

  });


  function displayEventsQuery(data)
  {
    var queryTable = document.getElementById("executeQueryTable");
    let i = 0;
    let j = 1;

    while(queryTable.rows.length > 0)
    {
      queryTable.deleteRow(0);
    }

    let queryRow1 = queryTable.insertRow(0);

    let eventIDCol = queryRow1.insertCell(0);
    let sumCol = queryRow1.insertCell(1);
    let startTimeCol = queryRow1.insertCell(2);
    let locaCol = queryRow1.insertCell(3);
    let orgCol = queryRow1.insertCell(4);
    let filRefCol = queryRow1.insertCell(5);

    eventIDCol.innerHTML = "Event ID";
    sumCol.innerHTML = "Summary";
    startTimeCol.innerHTML = "Start Time";
    locaCol.innerHTML = "Location";
    orgCol.innerHTML = "Organizer";
    filRefCol.innerHTML = "Calendar File";

    for(i = 0; i < data.length ; i++)
    {
      let queryRow1 = queryTable.insertRow(j);

      let eventIDCol = queryRow1.insertCell(0);
      let sumCol = queryRow1.insertCell(1);
      let startTimeCol = queryRow1.insertCell(2);
      let locaCol = queryRow1.insertCell(3);
      let orgCol = queryRow1.insertCell(4);
      let filRefCol = queryRow1.insertCell(5);

      eventIDCol.innerHTML = data[i].event_id;
      sumCol.innerHTML = data[i].summary;
      startTimeCol.innerHTML = data[i].start_time;
      locaCol.innerHTML = data[i].location;
      orgCol.innerHTML = data[i].organizer;
      filRefCol.innerHTML = data[i].cal_file;

      j++;
    }

  }
/**********************************************************QUERY # 2 *************************************************/
  $("#ExecuteQuery2").on("submit",function(event)
  {
    event.preventDefault();

    let eventFileObj = {
      fileName: $('#query2').val()
    };
    //Pass data to the Ajax call, so it gets passed to the
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
      data: eventFileObj,
      url: '/showPartEventsDB',   //The server endpoint we are connecting to
      success: function (data) {

        displayParticularEventsQuery(data);

      },
      error: function(error) {
          // Non-200 return, do something with error
          let parsedError = error.responseJSON;
          changeStatusPanel(parsedError.error);
      }
    }); //ajax bracket

  });

  function displayParticularEventsQuery(data)
  {
    var queryTable2 = document.getElementById("executeQueryTable");
    let i = 0;
    let j = 1;

    while(queryTable2.rows.length > 0)
    {
      queryTable2.deleteRow(0);
    }

    let queryRow1 = queryTable2.insertRow(0);

    let startTimeCol = queryRow1.insertCell(0);
    let sumCol = queryRow1.insertCell(1);

    startTimeCol.innerHTML = "Start Time";
    sumCol.innerHTML = "Summary";

    for(i = 0; i < data.length ; i++)
    {
      let queryRow1 = queryTable2.insertRow(j);

      let startTimeCol = queryRow1.insertCell(0);
      let sumCol = queryRow1.insertCell(1);

      startTimeCol.innerHTML = data[i].start_time;
      sumCol.innerHTML = data[i].summary;

      j++;
    }

  }






});
