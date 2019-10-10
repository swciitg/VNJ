package com.sai.chaithra.jntucejlibrary;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import java.util.Calendar;

public class AddBookDetails extends AppCompatActivity {

    Button book1;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_add_book_details);

        book1=findViewById(R.id.book1_reminder);
        book1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // long when = System.currentTimeMillis()+2*60*1000;         // notification time
// WHEN I RUN THE ABOVE COMMENTED CODE… THE REMINDER IS TRIGGERD AFTER EXACTLY 2 MINS
//BUT WHEN I USE THE BELOW CODE USING CALENDER INSTANCE, IT TRIGGER MY REMINDER    IMMIDIETLY WHEN I RUN IT ON MY DEVICE/EMULATOR


                Calendar calendar =  Calendar.getInstance();
                //calendar.set(2014,Calendar.getInstance().get(Calendar.MONTH),Calendar.SUNDAY , 8, 00, 00);
                calendar.set(2014,5,1,19,55,00);
                long when = calendar.getTimeInMillis();         // notification time



                Intent intentAlarm = new Intent(AddBookDetails.this, AlarmReceiver.class);

// create the object
                AlarmManager alarmManager = (AlarmManager)getSystemService(Context.ALARM_SERVICE);

                //set the alarm for particular time
                alarmManager.set(AlarmManager.RTC_WAKEUP,when, PendingIntent.getBroadcast(AddBookDetails.this,1,  intentAlarm, PendingIntent.FLAG_UPDATE_CURRENT));
            }
        });
    }
}
