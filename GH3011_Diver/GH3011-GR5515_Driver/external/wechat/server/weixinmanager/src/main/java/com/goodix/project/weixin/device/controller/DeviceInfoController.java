package com.goodix.project.weixin.device.controller;

import com.goodix.framework.aspectj.lang.annotation.Log;
import com.goodix.framework.aspectj.lang.enums.BusinessType;
import com.goodix.framework.config.WxMpConfiguration;
import com.goodix.framework.web.controller.BaseController;
import com.goodix.framework.web.domain.AjaxResult;
import com.goodix.framework.web.page.TableDataInfo;
import com.goodix.project.weixin.device.domain.DeviceInfo;
import com.goodix.project.weixin.device.service.IDeviceIInfoService;
import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.MultiFormatWriter;
import com.google.zxing.WriterException;
import com.google.zxing.client.j2se.MatrixToImageWriter;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.decoder.ErrorCorrectionLevel;
import me.chanjar.weixin.common.api.WxConsts;
import me.chanjar.weixin.common.error.WxErrorException;
import me.chanjar.weixin.mp.api.WxMpService;
import me.chanjar.weixin.mp.bean.card.BaseInfo;
import me.chanjar.weixin.mp.bean.device.*;
import me.chanjar.weixin.mp.constant.WxDeviceCloseStrategy;
import me.chanjar.weixin.mp.constant.WxDeviceConnectProtocol;
import me.chanjar.weixin.mp.constant.WxDeviceConnectionStrategy;
import me.chanjar.weixin.mp.constant.WxDeviceCryptMethod;
import org.apache.shiro.authz.annotation.RequiresPermissions;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Configuration;
import org.springframework.stereotype.Controller;
import org.springframework.ui.ModelMap;
import org.springframework.web.bind.annotation.*;

import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * @ProjectName: weixin
 * @Package: com.goodix.project.weixin.device.controller
 * @ClassName: DeviceInfoController
 * @Description: 后台硬件基础信息管理
 * @Author: liaoxiaohua
 * @Version: 1.0
 */
@Controller
@Configuration
@RequestMapping("/device/info")
public class DeviceInfoController extends BaseController {
    private String prefix = "weixin/device";

    private final Logger logger = LoggerFactory.getLogger(this.getClass());

    @Value("${wx.mp.cryptMethod}")
    private String cryptMethod;

    @Value("${wx.mp.authKey}")
    private String authKey;

    @Value("${wx.mp.authVer}")
    private String authVer;

    @Autowired
    private IDeviceIInfoService deviceIInfoService;
    private Object baseInfo;

    @RequiresPermissions("module:device:view")
    @GetMapping()
    public String info()
    {
        return prefix + "/info";
    }

    @GetMapping("/qrcode")
    public void qrCode(@RequestParam("ticket") String ticket,@RequestParam(name = "w",defaultValue = "200",required = false) int width,
                       @RequestParam(name = "h",defaultValue = "200",required = false) int height, @RequestParam(name = "f",defaultValue = "png",required = false) String format,HttpServletResponse response) throws IOException, WriterException {
        ServletOutputStream out = response.getOutputStream();
        Map<EncodeHintType,Object> config = new HashMap<>();
        config.put(EncodeHintType.CHARACTER_SET,"UTF-8");
        config.put(EncodeHintType.ERROR_CORRECTION, ErrorCorrectionLevel.M);
        config.put(EncodeHintType.MARGIN, 0);
        BitMatrix bitMatrix = new MultiFormatWriter().encode(ticket, BarcodeFormat.QR_CODE,width,height,config);
        MatrixToImageWriter.writeToStream(bitMatrix,format,out);
    }


    @RequiresPermissions("module:device:list")
    @PostMapping("/list")
    @ResponseBody
    public TableDataInfo list(DeviceInfo info)
    {
        startPage();
        List<DeviceInfo> list = deviceIInfoService.selectInfoList(info);
        return getDataTable(list);
    }


    @GetMapping("/add")
    public String add()
    {
        return prefix + "/add";
    }

    /**
     *
     * 功能描述: 调用微信接口获取设备二维码并授权设备以及保存硬件基础信息
     *
     * @param: info 硬件基础信息
     * @return: ajaxResult 处理结果信息
     * @author: liaoxiaohua
     */
    @RequiresPermissions("module:device:add")
    @Log(title = "硬件基础", businessType = BusinessType.INSERT)
    @PostMapping("/add")
    @ResponseBody
    public AjaxResult addSave(DeviceInfo info)
    {
        //设备二维码获取及设备授权接口请查看微信硬件平台文档https://iot.weixin.qq.com/wiki/new/index.html?page=3-4-6

        final WxMpService wxService = WxMpConfiguration.getMpServices().get(info.getAppId());
        if(wxService!=null){
            try {
                WxDeviceQrCodeResult qrCodeResult = wxService.getDeviceService().getQrCode(info.getProductId());
                if(qrCodeResult.getBaseResp().getErrCode()==0){
                    info.setDeviceId(qrCodeResult.getDeviceId());
                    info.setDeviceQrcode(qrCodeResult.getQrTicket());
                    info.setDeviceType("");
                    deviceIInfoService.insertInfo(info);
                    WxDeviceAuthorize deviceAuthorize = new WxDeviceAuthorize();
                    deviceAuthorize.setDeviceNum("1");
                    deviceAuthorize.setOpType("1");
                    deviceAuthorize.setProductId(info.getProductId());
                    List<WxDevice> deviceList = new LinkedList<>();
                    WxDevice wxDevice = new WxDevice();
                    wxDevice.setId(qrCodeResult.getDeviceId());
                    wxDevice.setMac(info.getDeviceMac());
                    wxDevice.setConnectProtocol(WxDeviceConnectProtocol.BLE);
                    wxDevice.setAuthKey(authKey);
                    wxDevice.setAuthVer(authVer);
                    wxDevice.setCloseStrategy(WxDeviceCloseStrategy.CLOSE_CONNECTION);
                    wxDevice.setConnStrategy(WxDeviceConnectionStrategy.FRONT);
                    wxDevice.setCryptMethod(cryptMethod);
                    wxDevice.setManuMacPos("-1");
                    wxDevice.setSerMacPos("-2");
                    deviceList.add(wxDevice);
                    deviceAuthorize.setDeviceList(deviceList);
                    WxDeviceAuthorizeResult deviceAuthorizeResult=wxService.getDeviceService().authorize(deviceAuthorize);
                    List<BaseResp> baseRespList=deviceAuthorizeResult.getResp();
                    BaseResp authResp=baseRespList.get(0);
                    if(authResp.getErrCode()!=0){
                        return error("获取二维码成功,但授权失败"+authResp.getErrCode()+":"+authResp.getErrMsg());
                    }else{
                        info.setDeviceType(authResp.getDeviceType());
                        return  toAjax(deviceIInfoService.updateInfo(info));
                    }

                }else{
                   return error("获取二维码失败"+qrCodeResult.getBaseResp().getErrMsg());
                }
            } catch (WxErrorException e) {
                e.printStackTrace();
                return error(e.getMessage());
            }
        }else{
            return error("没有该产品");
        }

    }


    @GetMapping("/edit/{deviceId}")
    public String edit(@PathVariable("deviceId") String deviceId, ModelMap mmap)
    {
        DeviceInfo info = deviceIInfoService.selectInfoById(deviceId);
        mmap.put("info", info);
        return prefix + "/edit";
    }


    @RequiresPermissions("module:device:edit")
    @Log(title = "硬件基础", businessType = BusinessType.UPDATE)
    @PostMapping("/edit")
    @ResponseBody
    public AjaxResult editSave(DeviceInfo info)
    {
        return toAjax(deviceIInfoService.updateInfo(info));
    }

    @RequiresPermissions("module:device:remove")
    @Log(title = "硬件基础", businessType = BusinessType.DELETE)
    @PostMapping( "/remove")
    @ResponseBody
    public AjaxResult remove(String ids)
    {
        return toAjax(deviceIInfoService.deleteInfoByIds(ids));
    }

}
