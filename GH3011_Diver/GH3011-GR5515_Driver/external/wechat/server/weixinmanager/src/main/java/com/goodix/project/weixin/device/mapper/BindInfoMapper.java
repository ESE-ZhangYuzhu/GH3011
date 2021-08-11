package com.goodix.project.weixin.device.mapper;

import com.goodix.project.weixin.device.domain.BindInfo;

import java.util.List;

public interface BindInfoMapper {
        /**
         * 查询设备绑定信息
         *
         * @param deviceId 设备绑定ID
         * @return 设备绑定信息
         */
        public BindInfo selectInfoById(String deviceId);

        /**
         * 查询设备绑定列表
         *
         * @param info 设备绑定信息
         * @return 设备绑定集合
         */
        public List<BindInfo> selectInfoList(BindInfo info);

        /**
         * 新增设备绑定
         *
         * @param info 设备绑定信息
         * @return 结果
         */
        public int insertInfo(BindInfo info);

        /**
         * 修改设备绑定
         *
         * @param info 设备绑定信息
         * @return 结果
         */
        public int updateInfo(BindInfo info);

        /**
         * 删除设备绑定
         *
         * @param deviceId 设备绑定ID
         * @return 结果
         */
        public int deleteInfoById(String deviceId);

        /**
         * 批量删除设备绑定
         *
         * @param deviceIds 需要删除的数据ID
         * @return 结果
         */
        public int deleteInfoByIds(String[] deviceIds);

        public BindInfo selectInfoByInfo(BindInfo bindInfo);

    }
