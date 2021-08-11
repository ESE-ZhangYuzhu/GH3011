package com.goodix.project.system.config.controller;

import java.util.List;

import com.goodix.framework.config.WxMpConfiguration;
import me.chanjar.weixin.common.api.WxConsts;
import me.chanjar.weixin.common.error.WxErrorException;
import me.chanjar.weixin.mp.api.WxMpService;
import me.chanjar.weixin.mp.bean.device.WxDeviceMsg;
import me.chanjar.weixin.mp.bean.kefu.WxMpKefuMessage;
import org.apache.commons.codec.binary.Base64;
import org.apache.shiro.authz.annotation.RequiresPermissions;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.ModelMap;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseBody;
import com.goodix.common.utils.poi.ExcelUtil;
import com.goodix.framework.aspectj.lang.annotation.Log;
import com.goodix.framework.aspectj.lang.enums.BusinessType;
import com.goodix.framework.web.controller.BaseController;
import com.goodix.framework.web.domain.AjaxResult;
import com.goodix.framework.web.page.TableDataInfo;
import com.goodix.project.system.config.domain.Config;
import com.goodix.project.system.config.service.IConfigService;

/**
 * 参数配置 信息操作处理
 * 
 * @author goodix
 */
@Controller
@RequestMapping("/system/config")
public class ConfigController extends BaseController
{
    private final Logger logger = LoggerFactory.getLogger(this.getClass());
    private String prefix = "system/config";

    @Autowired
    private IConfigService configService;

    @RequiresPermissions("system:config:view")
    @GetMapping()
    public String config()
    {
        return prefix + "/config";
    }

    /**
     * 查询参数配置列表
     */
    @RequiresPermissions("system:config:list")
    @PostMapping("/list")
    @ResponseBody
    public TableDataInfo list(Config config)
    {
        final WxMpService wxService = WxMpConfiguration.getMpServices().get("wx3b2ab6bae2ed8a6b");
        WxDeviceMsg wxDeviceMsg=new WxDeviceMsg();
        wxDeviceMsg.setContent(Base64.encodeBase64String("hello".getBytes()));
        wxDeviceMsg.setDeviceType("gh_3df234b71503");
        wxDeviceMsg.setDeviceId("gh_3df234b71503_a8847e51142bbd29");
        wxDeviceMsg.setOpenId("ohVFx1rFZuyWCmnFc7Gpkf0FjCfw");
        try {
            this.logger.info("发送数据到设备");
            wxService.getDeviceService().transMsg(wxDeviceMsg);
        } catch (WxErrorException e) {
            this.logger.info("发送数据到设备失败{}",e.getMessage());
            e.printStackTrace();
        }
        startPage();
        List<Config> list = configService.selectConfigList(config);
        return getDataTable(list);
    }

    @Log(title = "参数管理", businessType = BusinessType.EXPORT)
    @RequiresPermissions("system:config:export")
    @PostMapping("/export")
    @ResponseBody
    public AjaxResult export(Config config)
    {
        List<Config> list = configService.selectConfigList(config);
        ExcelUtil<Config> util = new ExcelUtil<Config>(Config.class);
        return util.exportExcel(list, "config");
    }

    /**
     * 新增参数配置
     */
    @GetMapping("/add")
    public String add()
    {
        final WxMpService wxService = WxMpConfiguration.getMpServices().get("wx3b2ab6bae2ed8a6b");
        WxDeviceMsg wxDeviceMsg=new WxDeviceMsg();
        wxDeviceMsg.setContent(Base64.encodeBase64String("hello".getBytes()));
        wxDeviceMsg.setDeviceType("gh_3df234b71503");
        wxDeviceMsg.setDeviceId("gh_3df234b71503_a8847e51142bbd29");
        wxDeviceMsg.setOpenId("ohVFx1rFZuyWCmnFc7Gpkf0FjCfw");
        try {
            this.logger.info("发送数据到设备");
            wxService.getDeviceService().transMsg(wxDeviceMsg);
        } catch (WxErrorException e) {
            this.logger.info("发送数据到设备失败{}",e.getMessage());
            e.printStackTrace();
        }
        return prefix + "/add";
    }

    /**
     * 新增保存参数配置
     */
    @RequiresPermissions("system:config:add")
    @Log(title = "参数管理", businessType = BusinessType.INSERT)
    @PostMapping("/add")
    @ResponseBody
    public AjaxResult addSave(Config config)
    {
        return toAjax(configService.insertConfig(config));
    }

    /**
     * 修改参数配置
     */
    @GetMapping("/edit/{configId}")
    public String edit(@PathVariable("configId") Long configId, ModelMap mmap)
    {
        mmap.put("config", configService.selectConfigById(configId));
        return prefix + "/edit";
    }

    /**
     * 修改保存参数配置
     */
    @RequiresPermissions("system:config:edit")
    @Log(title = "参数管理", businessType = BusinessType.UPDATE)
    @PostMapping("/edit")
    @ResponseBody
    public AjaxResult editSave(Config config)
    {
        return toAjax(configService.updateConfig(config));
    }

    /**
     * 删除参数配置
     */
    @RequiresPermissions("system:config:remove")
    @Log(title = "参数管理", businessType = BusinessType.DELETE)
    @PostMapping("/remove")
    @ResponseBody
    public AjaxResult remove(String ids)
    {
        return toAjax(configService.deleteConfigByIds(ids));
    }

    /**
     * 校验参数键名
     */
    @PostMapping("/checkConfigKeyUnique")
    @ResponseBody
    public String checkConfigKeyUnique(Config config)
    {
        return configService.checkConfigKeyUnique(config);
    }
}
