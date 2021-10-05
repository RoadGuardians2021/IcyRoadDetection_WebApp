<?php

//The marked out part is the ip address of the virtual machine and the password for the 'josh' user on phpMyAdmin

 $con = mysqli_connect('157.245.113.224','josh','34182057roadguardiansWSU','db'); //making the connection to the database

 if (!$con) {
     die('Connect Error:' . mysqli_connect_error()); //if connection fails, output why
 }
 else {
     echo 'Success...';
 }
 
/******************************************************************************************************************************************************************************************************************/

 $temp = '25.6';
 $curr = '-0.4';
 $device_id = "Device_1";

 $sql = "INSERT INTO Curr_Table (current, device_id, time_now) VALUES ($curr, '$device_id', CURRENT_TIMESTAMP())"; //inserting data into table (Curr_Table) made for holding the device's current sensor data

 
 if (mysqli_query($con, $sql)) {
    echo "<br>";
    echo "New record created successfully"; //if data cannot be inserted, output why
 }
 else {
     echo "Error: " . $sql . "<br>" . mysqli_error($con);
 }

/*******************************************************************************************************************************************************************************************************************/
 
 $sql = "INSERT INTO Temp_Table (temperature, device_id, time_now) VALUES ($temp, '$device_id', CURRENT_TIMESTAMP())"; //inserting data into table (Temp_Table) made for holding device's temperature sensor data

 if (mysqli_query($con, $sql)) {
    echo "<br>";
    echo "New record created successfully"; //if data cannot be inserted, output why
 }
 else {
     echo "Error: " . $sql . "<br>" . mysqli_error($con);
 }

/***************************************************************************************************************************************************************************************************************/

 $sql = "DELETE FROM Temp_Table WHERE time_now < DATE_SUB(NOW(), INTERVAL 1 DAY)"; //delete data from table (Temp_Table) after it has been in there for 24 hours

 if (mysqli_query($con, $sql)) {
    echo "<br>";
    echo "No errors";
 }
 else {
     echo "Error: " . $sql . "<br>" . mysqli_error($con);
 }

 /**************************************************************************************************************************************************************************************************************/

 $sql = "DELETE FROM Curr_Table WHERE time_now < DATE_SUB(NOW(), INTERVAL 1 DAY)"; //delete data from table (Curr_Table) after it has been in there for 24 hours

 if (mysqli_query($con, $sql)) {
    echo "<br>";
    echo "No errors";
 }
 else {
     echo "Error: " . $sql . "<br>" . mysqli_error($con);
 }

 mysqli_close($con);
 
?>