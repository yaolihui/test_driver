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
package com.fingerprints.navigationtest;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseExpandableListAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.fingerprints.navigationtest.helpers.LogItem;

import java.util.List;

public class LogItemAdapter extends BaseExpandableListAdapter {
    private Context mContext;
    private List<LogItem> mLogItems;

    private class GroupViewHolder {
        public TextView name;
        public TextView value;
        public ImageView indicatorImage;
    }

    public LogItemAdapter(final Context context, List<LogItem> listDataHeader) {
        mContext = context;
        mLogItems = listDataHeader;
    }

    @Override
    public Object getChild(final int groupPosition, final int childPosition) {
        return mLogItems.get(groupPosition).getItems().get(childPosition);
    }

    @Override
    public long getChildId(final int groupPosition, final int childPosition) {
        return childPosition;
    }

    @Override
    public View getChildView(final int groupPosition, final int childPosition, final boolean isLastChild, View convertView, final ViewGroup parent) {
        GroupViewHolder holder = null;
        if (convertView == null) {

            LayoutInflater vi = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = vi.inflate(R.layout.log_list_item_child, null);
            holder = new GroupViewHolder();
            holder.name = (TextView) convertView.findViewById(R.id.log_list_item_name);
            holder.value = (TextView) convertView.findViewById(R.id.log_list_item_value);

            convertView.setTag(holder);

        } else {
            holder = (GroupViewHolder) convertView.getTag();
        }

        LogItem item = (LogItem) getChild(groupPosition, childPosition);

        holder.name.setText(item.getHeader());
        holder.value.setText(item.getValue());

        return convertView;
    }

    @Override
    public int getChildrenCount(final int groupPosition) {
        return mLogItems.get(groupPosition).getItems().size();
    }

    @Override
    public Object getGroup(final int groupPosition) {
        return mLogItems.get(groupPosition);
    }

    @Override
    public int getGroupCount() {
        return mLogItems.size();
    }

    @Override
    public long getGroupId(final int groupPosition) {
        return groupPosition;
    }

    @Override
    public View getGroupView(final int groupPosition, final boolean isExpanded, View convertView, final ViewGroup parent) {
        GroupViewHolder holder = null;
        if (convertView == null) {

            LayoutInflater vi = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = vi.inflate(R.layout.log_list_item, null);
            holder = new GroupViewHolder();
            holder.name = (TextView) convertView.findViewById(R.id.log_list_item_name);
            holder.value = (TextView) convertView.findViewById(R.id.log_list_item_value);
            holder.indicatorImage = (ImageView) convertView.findViewById(R.id.log_list_item_group_indicator);
            convertView.setTag(holder);

        } else {
            holder = (GroupViewHolder) convertView.getTag();
        }

        LogItem item = (LogItem) getGroup(groupPosition);

        holder.name.setText(item.getHeader());
        holder.value.setText(item.getValue());

        if (getChildrenCount(groupPosition) == 0) {
            holder.indicatorImage.setVisibility(View.INVISIBLE);
        } else {
            holder.indicatorImage.setVisibility(View.VISIBLE);
            holder.indicatorImage.setImageResource(isExpanded ? R.drawable.ic_keyboard_arrow_down_black_24dp : R.drawable.ic_keyboard_arrow_right_black_24dp);
        }

        return convertView;
    }

    @Override
    public boolean hasStableIds() {
        return false;
    }

    @Override
    public boolean isChildSelectable(final int groupPosition, final int childPosition) {
        return false;
    }
}
