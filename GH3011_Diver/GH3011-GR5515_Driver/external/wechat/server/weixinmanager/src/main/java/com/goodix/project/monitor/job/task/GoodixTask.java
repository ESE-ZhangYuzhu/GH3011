package com.goodix.project.monitor.job.task;

import org.springframework.stereotype.Component;

/**
 * 定时任务调度测试
 * 
 * @author goodix
 */
@Component("goodixTask")
public class GoodixTask
{
    public void ryParams(String params)
    {
        System.out.println("执行有参方法：" + params);
    }

    public void ryNoParams()
    {
        System.out.println("执行无参方法");
    }
}
