package com.goodix.project.weixin.device.service;

import java.util.List;

import com.goodix.project.weixin.device.domain.DeviceInfo;
import com.goodix.project.weixin.device.mapper.DeviceInfoMapper;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import com.goodix.common.support.Convert;

/**
 * 硬件基础 服务层实现
 * 
 * @author goodix
 * @date 2018-11-05
 */
@Service
public class DeviceInfoServiceImpl implements IDeviceIInfoService
{
	@Autowired
	private DeviceInfoMapper deviceInfoMapper;

	/**
     * 查询硬件基础信息
     * 
     * @param deviceId 硬件基础ID
     * @return 硬件基础信息
     */
    @Override
	public DeviceInfo selectInfoById(String deviceId)
	{
	    return deviceInfoMapper.selectInfoById(deviceId);
	}
	
	/**
     * 查询硬件基础列表
     * 
     * @param info 硬件基础信息
     * @return 硬件基础集合
     */
	@Override
	public List<DeviceInfo> selectInfoList(DeviceInfo info)
	{
	    return deviceInfoMapper.selectInfoList(info);
	}
	
    /**
     * 新增硬件基础
     * 
     * @param info 硬件基础信息
     * @return 结果
     */
	@Override
	public int insertInfo(DeviceInfo info)
	{
	    return deviceInfoMapper.insertInfo(info);
	}
	
	/**
     * 修改硬件基础
     * 
     * @param info 硬件基础信息
     * @return 结果
     */
	@Override
	public int updateInfo(DeviceInfo info)
	{
	    return deviceInfoMapper.updateInfo(info);
	}

	/**
     * 删除硬件基础对象
     * 
     * @param ids 需要删除的数据ID
     * @return 结果
     */
	@Override
	public int deleteInfoByIds(String ids)
	{
		return deviceInfoMapper.deleteInfoByIds(Convert.toStrArray(ids));
	}
	
}
