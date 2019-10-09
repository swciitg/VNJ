package com.sai.chaithra.jntucejlibrary;

import android.content.Intent;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.AuthResult;
import com.google.firebase.auth.FirebaseAuth;

public class LoginPage extends AppCompatActivity {
     TextView RollNo;
     TextView Password;
     Button Login;
    private EditText inputEmail, inputPassword;
    private FirebaseAuth auth;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login_page);

        auth = FirebaseAuth.getInstance();
        if (auth.getCurrentUser() != null) {
            startActivity(new Intent(LoginPage.this, HomeActivity.class));
            finish();
        }
        inputEmail = (EditText) findViewById(R.id.email);
        inputPassword = (EditText) findViewById(R.id.password);
        Button login;
        login = findViewById(R.id.login);

                login.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        String email = inputEmail.getText().toString();
                        final String password = inputPassword.getText().toString();

                        if (TextUtils.isEmpty(email)) {
                            Toast.makeText(getApplicationContext(), "Enter email address!", Toast.LENGTH_SHORT).show();
                            return;
                        }
                        if(TextUtils.isEmpty(password)){
                            Toast.makeText(LoginPage.this, "Enter Password", Toast.LENGTH_SHORT).show();
                        }

                        auth.signInWithEmailAndPassword(email,password).addOnCompleteListener(LoginPage.this, new OnCompleteListener<AuthResult>() {
                            @Override
                            public void onComplete(@NonNull Task<AuthResult> task) {
                                if (!task.isSuccessful()) {
                                    // there was an error
                                    if (password.length() < 6) {
                                        inputPassword.setError("Minimum Length is 6 letters");
                                    } else {
                                        Toast.makeText(LoginPage.this, "Wrong Email/Password", Toast.LENGTH_LONG).show();
                                    }
                                } else {
                                    Intent intent = new Intent(LoginPage.this, HomeActivity.class);
                                    startActivity(intent);
                                    finish();
                                }
                            }
                        });
                    }
                });

    }
    public void MyCustomAlertDialog(){

    }
}
