package com.goodix.project.weixin.device.controller;

import com.goodix.framework.aspectj.lang.annotation.Log;
import com.goodix.framework.aspectj.lang.enums.BusinessType;
import com.goodix.framework.web.controller.BaseController;
import com.goodix.framework.web.domain.AjaxResult;
import com.goodix.framework.web.page.TableDataInfo;
import com.goodix.project.weixin.device.domain.BindInfo;
import com.goodix.project.weixin.device.service.IBindInfoService;
import org.apache.shiro.authz.annotation.RequiresPermissions;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.ModelMap;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * @ProjectName: weixin
 * @Package: com.goodix.project.weixin.device.controller
 * @ClassName: BindInfoController
 * @Description: 已绑定的硬件信息管理
 * @Author: liaoxiaohua
 * @Version: 1.0
 */
@Controller
@RequestMapping("/bind/info")
public class BindInfoController extends BaseController
{
    private String prefix = "weixin/bind";

    @Autowired
    private IBindInfoService bindInfoService;

    @RequiresPermissions("module:bind:view")
    @GetMapping()
    public String info()
    {
        return prefix + "/info";
    }


    @RequiresPermissions("module:bind:list")
    @PostMapping("/list")
    @ResponseBody
    public TableDataInfo list(BindInfo info)
    {
        startPage();
        List<BindInfo> list = bindInfoService.selectInfoList(info);
        return getDataTable(list);
    }

    @GetMapping("/add")
    public String add()
    {
        return prefix + "/add";
    }


    @RequiresPermissions("module:bind:add")
    @Log(title = "设备绑定", businessType = BusinessType.INSERT)
    @PostMapping("/add")
    @ResponseBody
    public AjaxResult addSave(BindInfo info)
    {
        return toAjax(bindInfoService.insertInfo(info));
    }


    @GetMapping("/edit/{deviceId}")
    public String edit(@PathVariable("deviceId") String deviceId, ModelMap mmap)
    {
        BindInfo info = bindInfoService.selectInfoById(deviceId);
        mmap.put("info", info);
        return prefix + "/edit";
    }


    @RequiresPermissions("module:bind:edit")
    @Log(title = "设备绑定", businessType = BusinessType.UPDATE)
    @PostMapping("/edit")
    @ResponseBody
    public AjaxResult editSave(BindInfo info)
    {
        return toAjax(bindInfoService.updateInfo(info));
    }


    @RequiresPermissions("module:bind:remove")
    @Log(title = "设备绑定", businessType = BusinessType.DELETE)
    @PostMapping( "/remove")
    @ResponseBody
    public AjaxResult remove(String ids)
    {
        return toAjax(bindInfoService.deleteInfoByIds(ids));
    }

}