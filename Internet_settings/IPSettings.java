package com.swc.onestop.Internet_settings;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

import com.swc.onestop.Activities.Internet_settings;
import com.swc.onestop.Activities.Main2Activity;
import com.swc.onestop.R;

public class IPSettings extends Activity {
    private Spinner hostel;
    private EditText block, floor, room;
    private TextView message, ipAddress, subnet, gateway, dns;
    private Button submit;
    private int selectedHostel = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ipsettings);


        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        toolbar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // back button pressed
                startActivity(new Intent(IPSettings.this,Main2Activity.class));
                finish();
            }
        });
        initialize();
        message.setText("Please fill in the above details!");
    }

    private void initialize() {
        String[] Hostels = {"Select hostel", "Barak", "Brahmaputra", "Dhansiri", "Dibang", "Dihing", "Kameng", "Kapili", "Lohit", "Manas", "Married Scholars", "Siang", "Subansiri", "Umiam"};

        hostel = (Spinner) findViewById(R.id.SHostel);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, Hostels);
        hostel.setAdapter(adapter);
        hostel.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                subnet.setVisibility(View.GONE);
                ipAddress.setVisibility(View.GONE);
                gateway.setVisibility(View.GONE);
                selectedHostel = i;
                calculateDetails();
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {
                selectedHostel = 0;
            }
        });

        block = (EditText) findViewById(R.id.etBlock);
        floor = (EditText) findViewById(R.id.etFloor);
        room = (EditText) findViewById(R.id.etRoom);

        message = (TextView) findViewById(R.id.tvIPMessage);
        ipAddress = (TextView) findViewById(R.id.tvIPAddress);
        subnet = (TextView) findViewById(R.id.tvSubnet);
        gateway = (TextView) findViewById(R.id.tvGateway);
        dns = (TextView) findViewById(R.id.tvDNS);

        submit = (Button) findViewById(R.id.bIPSubmit);
        submit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                calculateDetails();
            }
        });
    }

    private void calculateDetails() {
        String blockNumber = block.getText().toString().toLowerCase();
        String floorNumber = floor.getText().toString().toLowerCase();
        String roomNumber = room.getText().toString().toLowerCase();

        subnet.setVisibility(View.GONE);
        ipAddress.setVisibility(View.GONE);
        gateway.setVisibility(View.GONE);

        if (selectedHostel != 0 && blockNumber.length() != 0 && floorNumber.length() != 0 && roomNumber.length() != 0) {
            if (selectedHostel == 3 || selectedHostel == 8 || selectedHostel == 10) {
                message.setText("You don't have to set your IP address, it is done automatically.");
            } else if (selectedHostel == 4) {
                message.setText("Please visit http://www.iitg.ernet.in/interiitsports/hostelNetSettings.htm for further details.");
            } else {
                message.setText("Your details:");
                subnet.setVisibility(View.VISIBLE);
                ipAddress.setVisibility(View.VISIBLE);
                gateway.setVisibility(View.VISIBLE);
                String[] start = {"", "10.10", "10.12", "", "", "10.0", "10.9", "10.1", "", "10.4", "", "10.3", "10.16", "10.11"};

                if (selectedHostel == 5 || selectedHostel == 7 || selectedHostel == 11) {
                    subnet.setText("Subnet: 255.255.252.0");
                } else {
                    subnet.setText("Subnet: 255.255.192.0");
                }

                gateway.setText("Gateway: " + start[selectedHostel] + ".0.254");

                int W = -1, X = -1, Y = -1, Z = -1;
                switch (blockNumber) {
                    case "b1":
                        W = 0;
                        break;
                    case "b2":
                        W = 1;
                        break;
                    case "b3":
                        W = 2;
                        break;
                    case "b4":
                        W = 3;
                        break;
                    case "c1":
                        W = 4;
                        break;
                    default:
                        W = blockNumber.getBytes()[0] - 'a';
                        break;
                }

                X = Integer.parseInt(floorNumber);
                Y = Integer.parseInt(roomNumber);

                if (selectedHostel == 2) {
                    if (W == 0 || W == 1) {
                        Z = Y;
                    } else {
                        if (X == 0) {
                            Z = Y - 134;
                        } else if (X == 1) {
                            Z = Y - 119;
                        } else if (X == 2) {
                            Z = Y - 133;
                        } else {
                            Z = Y - 131;
                        }
                    }
                } else {
                    Y %= 100;
                }

                if (selectedHostel == 5 || selectedHostel == 7 || selectedHostel == 11) {
                    ipAddress.setText("IP Address: " + start[selectedHostel] + "." + X + "." + Y);
                } else if (selectedHostel == 2) {
                    ipAddress.setText("IP Address: " + start[selectedHostel] + "." + W + X + "." + Z);
                } else {
                    ipAddress.setText("IP Address: " + start[selectedHostel] + "." + W + X + "." + Y);
                }
            }
        } else {
            if (selectedHostel == 3 || selectedHostel == 8 || selectedHostel == 10) {
                message.setText("You don't have to set your IP address, it is done automatically.");
            } else if (selectedHostel == 4) {
                message.setText("Please visit http://www.iitg.ernet.in/interiitsports/hostelNetSettings.htm for further details.");
            } else {
                message.setText("Please fill in the above details!");
            }
        }
    }
}
