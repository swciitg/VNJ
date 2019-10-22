package com.swc.onestop.Internet_settings;

import android.app.Activity;
import android.content.Intent;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import com.google.android.gms.vision.L;
import com.swc.onestop.Activities.Internet_settings;
import com.swc.onestop.Activities.Main2Activity;
import com.swc.onestop.R;

public class LanSet extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.lanset);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        toolbar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // back button pressed
                startActivity(new Intent(LanSet.this,Main2Activity.class));
                finish();
            }
        });





    }
}
