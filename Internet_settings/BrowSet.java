package com.swc.onestop.Internet_settings;

import android.app.Activity;
import android.content.Intent;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TabHost;
import android.widget.TabHost.TabSpec;

import com.swc.onestop.Activities.Internet_settings;
import com.swc.onestop.Activities.Main2Activity;
import com.swc.onestop.R;

public class BrowSet extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.browset);

		Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
		toolbar.setNavigationOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// back button pressed
				startActivity(new Intent(BrowSet.this,Main2Activity.class));
				finish();
			}
		});
		
		TabHost th = (TabHost) findViewById(R.id.tabhost);
		th.setup();
		
		TabSpec ts = th.newTabSpec("tag1");
		ts.setContent(R.id.tab1);
		ts.setIndicator("Chrome");
		th.addTab(ts);
		
		ts = th.newTabSpec("tag2");
		ts.setContent(R.id.tab2);
		ts.setIndicator("Firefox");
		th.addTab(ts);
	}
}
