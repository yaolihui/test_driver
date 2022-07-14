/*
 *
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
package com.fingerprints.sensortesttool;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.fingerprints.sensortesttool.testcases.ATestCase;
import com.fingerprints.sensortesttool.testcases.SensorTestCase;

import java.util.ArrayList;

public class TestCaseAdapter extends ArrayAdapter<ATestCase> {

    private boolean mEnabled = true;

    public TestCaseAdapter(Context context, int resourceId, ArrayList<ATestCase> testCases) {
        super(context, resourceId, testCases);
    }

    public boolean isEnabled() {
        return mEnabled;
    }

    public void setEnabled(final boolean enabled) {
        mEnabled = enabled;
        notifyDataSetChanged();
    }

    private class ViewHolder {
        public TextView name;
        public ImageView statusImage;
        public CheckBox selected;
        public ImageView expandImage;
        public RelativeLayout infoLayout1;
        public TextView description;
        public RelativeLayout descriptionLayout;
        public TextView infoText1;
    }


    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

        ViewHolder holder = null;

        if (convertView == null) {
            LayoutInflater vi = (LayoutInflater) getContext().getSystemService(
                    Context.LAYOUT_INFLATER_SERVICE);
            convertView = vi.inflate(R.layout.testcase_item, null);

            holder = new ViewHolder();
            holder.selected = (CheckBox) convertView.findViewById(R.id.testcase_checkbox);
            holder.name = (TextView) convertView.findViewById(R.id.testcase_name);
            holder.statusImage = (ImageView) convertView.findViewById(R.id.testcase_status_image);
            holder.expandImage = (ImageView) convertView.findViewById(R.id.testcase_expand);
            holder.infoLayout1 = (RelativeLayout) convertView.findViewById(R.id.testcase_info_layout_1);
            holder.infoText1 = (TextView) convertView.findViewById(R.id.testcase_info_text_1);
            holder.descriptionLayout = (RelativeLayout) convertView.findViewById(R.id.testcase_description);
            holder.description = (TextView) convertView.findViewById(R.id.testcase_description_text);
            convertView.setTag(holder);

            final CheckBox checkBox = holder.selected;

            holder.selected.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(final View v) {
                    CheckBox cb = (CheckBox) v;
                    ATestCase testCase = (ATestCase) cb.getTag();
                    testCase.setSelected(cb.isChecked());
                    testCase.setStatus(TestStatus.STOPPED);
                }
            });

        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        ATestCase testCase = getItem(position);
        holder.name.setText(testCase.getName());
        holder.selected.setSelected(testCase.isSelected());

        if (testCase.getStatus().isPassed()) {
            holder.statusImage.setImageDrawable(getContext().getDrawable(R.drawable.icon_passed));
        } else if (testCase.getStatus().isFailed()) {
            holder.statusImage.setImageDrawable(getContext().getDrawable(R.drawable.icon_error));
        } else if (testCase.getStatus().isCancelled()) {
            holder.statusImage.setImageDrawable(getContext().getDrawable(R.drawable.icon_cancel));
        } else if (testCase.getStatus().isWaiting()) {
            holder.statusImage.setImageDrawable(getContext().getDrawable(R.drawable.icon_waiting));
        } else {
            holder.statusImage.setImageBitmap(null);
        }

        if (testCase instanceof SensorTestCase) {
            SensorTestCase sensorTest = (SensorTestCase) testCase;
            String stampType = sensorTest.getSensorTest().rubberStampType;
            if (stampType != null && !stampType.equals("") && testCase.isManual()) {
                holder.infoLayout1.setVisibility(View.VISIBLE);
                holder.infoText1.setText(stampType);
            } else {
                holder.infoLayout1.setVisibility(View.GONE);
            }
        } else {
            holder.infoLayout1.setVisibility(View.GONE);
        }

        String description = testCase.getDescription();
        if (description != null && !description.equals("")) {
            holder.descriptionLayout.setVisibility(View.VISIBLE);

            if (!description.endsWith(".")) {
                description += ".";
            }

            holder.description.setText(description);
        } else {
            holder.descriptionLayout.setVisibility(View.GONE);
        }


        if (testCase.hasResult()) {
            holder.expandImage.setVisibility(View.VISIBLE);
        } else {
            holder.expandImage.setVisibility(View.INVISIBLE);
        }

        holder.selected.setTag(testCase);
        holder.selected.setChecked(testCase.isSelected());
        holder.selected.setEnabled(mEnabled);

        return convertView;
    }
}
