package com.sai.chaithra.jntucejlibrary;

import android.content.Intent;
import android.net.wifi.p2p.WifiP2pManager;
import android.support.annotation.NonNull;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.transition.TransitionManager;
import android.view.Gravity;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Toolbar;

import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;

public class HomeActivity extends AppCompatActivity implements NavigationView.OnNavigationItemSelectedListener{
    private DrawerLayout mJntuhcejLibrary ;
    Button BOOK1 ;
    Button BOOK2 ;
    Button BOOK3 ;

    private FirebaseAuth auth;
      @Override
    protected void onCreate(Bundle savedInstanceState) {
          super.onCreate(savedInstanceState);
          setContentView(R.layout.activity_home);
          mJntuhcejLibrary = findViewById(R.id.drawer);
          NavigationView navigationView = findViewById(R.id.navigation_view);
          navigationView.setNavigationItemSelectedListener(this);
          navigationView.setItemIconTintList(null);
          ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(this, mJntuhcejLibrary, R.string.open, R.string.close);
          mJntuhcejLibrary.addDrawerListener(toggle);
          toggle.syncState();
          auth = FirebaseAuth.getInstance();
          getSupportActionBar().setDisplayHomeAsUpEnabled(true);

          FloatingActionButton Plus=findViewById(R.id.fab);
          Plus.setOnClickListener(new View.OnClickListener() {
              @Override
              public void onClick(View v) {
                Intent i=new Intent(HomeActivity.this,AddBookDetails.class);
                startActivity(i);
              }
          });

          final ViewGroup transitionsContainer = findViewById(R.id.transitions_container);
          final TextView text = (TextView) transitionsContainer.findViewById(R.id.text);
          final Button button = (Button) transitionsContainer.findViewById(R.id.button);

          button.setOnClickListener(new View.OnClickListener() {
              boolean visible;
              @Override
              public void onClick(View v) {

                  TransitionManager.beginDelayedTransition(transitionsContainer);
                  visible = !visible;
                  text.setVisibility(visible ? View.VISIBLE : View.GONE);
              }
          });


          final ViewGroup transitionsContainer2 = findViewById(R.id.transitions_container2);
          final TextView text2 = (TextView) transitionsContainer2.findViewById(R.id.text2);
          final Button button2 = (Button) transitionsContainer2.findViewById(R.id.button2);

          button2.setOnClickListener(new View.OnClickListener() {
              boolean visible;
              @Override
              public void onClick(View v) {

                  TransitionManager.beginDelayedTransition(transitionsContainer2);
                  visible = !visible;
                  text2.setText("Chaitra");
                  text2.setVisibility(visible ? View.VISIBLE : View.GONE);
              }
          });

          final ViewGroup transitionsContainer3 = findViewById(R.id.transitions_container3);
          final TextView text3 = (TextView) transitionsContainer3.findViewById(R.id.text3);
          final Button button3 = (Button) transitionsContainer3.findViewById(R.id.button3);

          button3.setOnClickListener(new View.OnClickListener() {
              boolean visible;
              @Override
              public void onClick(View v) {

                  TransitionManager.beginDelayedTransition(transitionsContainer3);
                  visible = !visible;
                  text3.setVisibility(visible ? View.VISIBLE : View.GONE);
              }
          });
      }
    @Override
    public void onBackPressed() {
     if (mJntuhcejLibrary.isDrawerOpen(GravityCompat.START)) {
         mJntuhcejLibrary.closeDrawer(GravityCompat.START);
     } else {
         super.onBackPressed();
     }

    }

    @Override
    public boolean onNavigationItemSelected(@NonNull MenuItem menuItem) {

        int id = menuItem.getItemId();

        if (id == R.id.MB) {
            Toast.makeText(this, "My Books CLicked", Toast.LENGTH_SHORT).show();
        }else if( id == R.id.notifications){
            Toast.makeText(this, "Notifications CLicked", Toast.LENGTH_SHORT).show();
        }
        else if(id == R.id.settings){
            Toast.makeText(this, "Settings CLicked", Toast.LENGTH_SHORT).show();

        }else if(id == R.id.logout){
            auth.signOut();
            startActivity(new Intent(HomeActivity.this, LoginPage.class));
            Toast.makeText(HomeActivity.this, "Logged Out", Toast.LENGTH_SHORT).show();

// this listener will be called when there is change in firebase user session
            FirebaseAuth.AuthStateListener authListener = new FirebaseAuth.AuthStateListener() {
                @Override
                public void onAuthStateChanged(@NonNull FirebaseAuth firebaseAuth) {
                    FirebaseUser user = firebaseAuth.getCurrentUser();
                    if (user == null) {
                        // user auth state is changed - user is null
                        // launch login activity
                        finish();
                    }
                }
            };
        }

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer);
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        if (mJntuhcejLibrary.isDrawerOpen(Gravity.LEFT)) {
            mJntuhcejLibrary.closeDrawer(Gravity.LEFT);
        } else {
            mJntuhcejLibrary.openDrawer(Gravity.LEFT);
        }
        return super.onOptionsItemSelected(item);
    }

}

