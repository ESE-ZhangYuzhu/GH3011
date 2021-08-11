package com.goodix.project.weixin.device.service;

import com.goodix.project.weixin.device.domain.BindInfo;

import java.util.List;

/**
 * 设备绑定 服务层
 * 
 * @author goodix
 * @date 2018-11-06
 */
public interface IBindInfoService
{
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
     * 删除设备绑定信息
     * 
     * @param ids 需要删除的数据ID
     * @return 结果
     */
	public int deleteInfoByIds(String ids);

	public BindInfo selectInfoByInfo(BindInfo bindInfo);
	
}
