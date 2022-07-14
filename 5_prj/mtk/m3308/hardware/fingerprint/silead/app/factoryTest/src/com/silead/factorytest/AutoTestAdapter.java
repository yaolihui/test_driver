
package com.silead.factorytest;

import android.content.Context;
import android.graphics.Color;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.ArrayList;

public class AutoTestAdapter extends BaseAdapter {
    public static class ItemData {
        public String name;
        public String desc;
        public int result = -1;

        public ItemData(String name) {
            this(name, -1);
        }

        public ItemData(String name, int result) {
            this(name, null, result);
        }

        public ItemData(String name, String desc, int result) {
            this.name = name;
            this.result = result;
            this.desc = desc;
        }

        public void setItemResult(int result) {
            this.result = result;
        }

        public void setItemDesc(String desc) {
            this.desc = desc;
        }

        public String getItemName() {
            return name;
        }

        public String getItemDesc() {
            return desc;
        }

        public boolean isItemPass() {
            return result > 0;
        }

        public boolean isItemResultValid() {
            return result >= 0;
        }
    }

    public static class ViewHolder {
        TextView tv_name;
        TextView tv_result;
    }

    private Context context = null;
    private ArrayList<ItemData> data_list;

    public AutoTestAdapter(Context context) {
        this.context = context;
    }

    void setDataList(ArrayList<ItemData> list) {
        data_list = list;
    }

    @Override
    public int getCount() {
        return data_list.size();
    }

    @Override
    public Object getItem(int i) {
        return data_list.get(i);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder mHolder;
        if (convertView == null) {
            mHolder = new ViewHolder();
            LayoutInflater inflater = LayoutInflater.from(context);
            convertView = inflater.inflate(R.layout.test_auto_item, null, true);
            mHolder.tv_name = (TextView) convertView.findViewById(R.id.test_auto_item_name);
            mHolder.tv_result = (TextView) convertView.findViewById(R.id.test_auto_item_result);
            convertView.setTag(mHolder);
        } else {
            mHolder = (ViewHolder) convertView.getTag();
        }

        ItemData item = data_list.get(position);
        if (item != null) {
            mHolder.tv_name.setText(item.getItemName());
            if (item.isItemResultValid()) {
                if (item.isItemPass()) {
                    mHolder.tv_result.setText("PASS");
                    mHolder.tv_result.setTextColor(Color.GREEN);
                } else {
                    mHolder.tv_result.setText("FAIL");
                    mHolder.tv_result.setTextColor(Color.RED);
                }
            } else {
                String desc = item.getItemDesc();
                if (desc != null) {
                    mHolder.tv_result.setText(desc);
                } else {
                    mHolder.tv_result.setText("");
                }
            }
        }
        return convertView;
    }
}