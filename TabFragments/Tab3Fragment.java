package com.swc.onestop.TabFragments;


import android.Manifest;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Toast;


import com.swc.onestop.EachFragmentMenu.EmergencyMenu;
import com.swc.onestop.EachFragmentMenu.EmergencyMenuAdapter;
import com.swc.onestop.R;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by User on 2/28/2017.
 */

public class Tab3Fragment extends Fragment {
    private static final String TAG = "Tab1Fragment";

    private RecyclerView mRecyclerView;
    private RecyclerView.Adapter mAdapter;
    private RecyclerView.LayoutManager mLayoutManager;
    private List<EmergencyMenu> myDataset;
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.tab3_fragment,container,false);


        myDataset = new ArrayList<>();
        myDataset.add(new EmergencyMenu("Security" ,
                "0361 2584444",
                getResources().getDrawable(R.drawable.security_illustration)));
        myDataset.add(new EmergencyMenu("Hospital" ,
                "0361 2585555",
                getResources().getDrawable(R.drawable.medical_illustration)));
        myDataset.add(new EmergencyMenu("Ambulance" ,
                "0361 2582099",
                getResources().getDrawable(R.drawable.medical_illustration)));

        mRecyclerView = view.findViewById(R.id.tab3recycler);
        mRecyclerView.setHasFixedSize(true);
        mLayoutManager = new LinearLayoutManager(getActivity());
        mRecyclerView.setLayoutManager(mLayoutManager);

        // specify an adapter (see also next example)
        mAdapter = new EmergencyMenuAdapter(myDataset, getActivity());
        mRecyclerView.setAdapter(mAdapter);
        return view;
    }
}

