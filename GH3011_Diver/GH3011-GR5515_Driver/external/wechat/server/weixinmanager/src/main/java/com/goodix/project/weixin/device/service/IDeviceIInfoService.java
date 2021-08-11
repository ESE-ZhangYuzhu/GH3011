package com.goodix.project.weixin.device.service;

import com.goodix.project.weixin.device.domain.DeviceInfo;
import java.util.List;

/**
 * 硬件基础 服务层
 * 
 * @author goodix
 * @date 2018-11-05
 */
public interface IDeviceIInfoService
{
	/**
     * 查询硬件基础信息
     * 
     * @param deviceId 硬件基础ID
     * @return 硬件基础信息
     */
	public DeviceInfo selectInfoById(String deviceId);
	
	/**
     * 查询硬件基础列表
     * 
     * @param info 硬件基础信息
     * @return 硬件基础集合
     */
	public List<DeviceInfo> selectInfoList(DeviceInfo info);
	
	/**
     * 新增硬件基础
     * 
     * @param info 硬件基础信息
     * @return 结果
     */
	public int insertInfo(DeviceInfo info);
	
	/**
     * 修改硬件基础
     * 
     * @param info 硬件基础信息
     * @return 结果
     */
	public int updateInfo(DeviceInfo info);
		
	/**
     * 删除硬件基础信息
     * 
     * @param ids 需要删除的数据ID
     * @return 结果
     */
	public int deleteInfoByIds(String ids);
	
}
