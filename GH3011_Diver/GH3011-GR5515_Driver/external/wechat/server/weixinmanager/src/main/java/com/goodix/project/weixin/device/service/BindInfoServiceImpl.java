package com.goodix.project.weixin.device.service;

import java.util.List;

import com.goodix.project.weixin.device.domain.BindInfo;
import com.goodix.project.weixin.device.mapper.BindInfoMapper;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import com.goodix.common.support.Convert;

/**
 * 设备绑定 服务层实现
 * 
 * @author goodix
 * @date 2018-11-06
 */
@Service
public class BindInfoServiceImpl implements IBindInfoService
{
	@Autowired
	private BindInfoMapper infoMapper;

	/**
     * 查询设备绑定信息
     * 
     * @param deviceId 设备绑定ID
     * @return 设备绑定信息
     */
    @Override
	public BindInfo selectInfoById(String deviceId)
	{
	    return infoMapper.selectInfoById(deviceId);
	}
	
	/**
     * 查询设备绑定列表
     * 
     * @param info 设备绑定信息
     * @return 设备绑定集合
     */
	@Override
	public List<BindInfo> selectInfoList(BindInfo info)
	{
	    return infoMapper.selectInfoList(info);
	}
	
    /**
     * 新增设备绑定
     * 
     * @param info 设备绑定信息
     * @return 结果
     */
	@Override
	public int insertInfo(BindInfo info)
	{
	    return infoMapper.insertInfo(info);
	}
	
	/**
     * 修改设备绑定
     * 
     * @param info 设备绑定信息
     * @return 结果
     */
	@Override
	public int updateInfo(BindInfo info)
	{
	    return infoMapper.updateInfo(info);
	}

	/**
     * 删除设备绑定对象
     * 
     * @param ids 需要删除的数据ID
     * @return 结果
     */
	@Override
	public int deleteInfoByIds(String ids)
	{
		return infoMapper.deleteInfoByIds(Convert.toStrArray(ids));
	}

	@Override
	public BindInfo selectInfoByInfo(BindInfo bindInfo) {
		return infoMapper.selectInfoByInfo(bindInfo);
	}

}
