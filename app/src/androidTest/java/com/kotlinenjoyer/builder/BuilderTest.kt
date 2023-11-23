package com.kotlinenjoyer.builder

import androidx.test.ext.junit.runners.AndroidJUnit4
import com.kotlinenjoer.builder.Builder
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class BuilderTest {
    @Test
    fun aso_build_is_correct() {
        val result = Builder.buildQuery(
            domain = "https://www.google.com?",
            params = mapOf(
                "bundle" to "com.dev.test",
                "af_userid" to "123456789",
            )
        )
        println("Result: $result")
        Assert.assertTrue(result.startsWith("https://www.google.com?"))
        Assert.assertTrue(result.contains("bundle=com.dev.test"))
        Assert.assertTrue(result.contains("af_userid=123456789"))
        Assert.assertTrue(result.contains("notId=null"))
        Assert.assertTrue(result.contains("sub10=firstOpen"))
        Assert.assertEquals(4, result.split("&").size)
        Assert.assertFalse(result.endsWith("&"))
        Assert.assertFalse(result.contains(" "))
    }

    @Test
    fun campaign_processing_is_correct() {
        val result = Builder.buildQuery(
            domain = "https://www.google.com?",
            params = mapOf(
                "bundle" to "com.dev.test",
                "af_userid" to "123456789",
                "campaign_1" to "myapp://test1_push_test2_test3_test4_test5_test6_test7_test8_test9_test10_test11",
                "campaign_2" to "a_b_c",
                "campaign_id" to "CAMPAIGN_ID 1",
            )
        )
        println("Result: $result")
        Assert.assertTrue(result.startsWith("https://www.google.com?"))
        Assert.assertTrue(result.contains("bundle=com.dev.test"))
        Assert.assertTrue(result.contains("af_userid=123456789"))
        Assert.assertTrue(result.contains("notId=null"))
        Assert.assertTrue(result.contains("sub10=firstOpen"))
        Assert.assertTrue(result.contains("campaign=myapp%3A%2F%2Ftest1_push_test2_test3_test4_test5_test6_test7_test8_test9_test10_test11"))
        Assert.assertTrue(result.contains("push=push"))
        Assert.assertTrue(result.contains("sub1=test1"))
        Assert.assertTrue(result.contains("sub11=test11"))
        Assert.assertTrue(result.contains("campaign_id=CAMPAIGN_ID%201"))
        Assert.assertFalse(result.endsWith("&"))
        Assert.assertFalse(result.contains(" "))
    }

    @Test
    fun empty_campaign_processing_is_correct() {
        val result = Builder.buildQuery(
            domain = "https://www.google.com?",
            params = mapOf(
                "bundle" to "com.dev.test",
                "af_userid" to "123456789",
                "campaign" to "null",
                "campaign_id" to "CAMPAIGN_ID 1",
            )
        )
        println("Result: $result")
        Assert.assertTrue(result.startsWith("https://www.google.com?"))
        Assert.assertTrue(result.contains("bundle=com.dev.test"))
        Assert.assertTrue(result.contains("af_userid=123456789"))
        Assert.assertTrue(result.contains("notId=null"))
        Assert.assertTrue(result.contains("sub10=firstOpen"))
        Assert.assertTrue(result.contains("campaign=null"))
        Assert.assertTrue(result.contains("push=null"))
        Assert.assertTrue(result.contains("sub1=null"))
        Assert.assertTrue(result.contains("sub4=&"))
        Assert.assertTrue(result.contains("sub5=&"))
        Assert.assertTrue(result.contains("campaign_id=CAMPAIGN_ID%201"))
        Assert.assertFalse(result.contains("sub11="))
        Assert.assertFalse(result.endsWith("&"))
        Assert.assertFalse(result.contains(" "))
    }

    @Test
    fun get_value_is_correct() {
        val data = "https://www.google.com?bundle=com.a.b&push=test&sub1=tt"
        val result = Builder.getParamValue(data, "push")

        Assert.assertEquals("test", result)
    }

    @Test
    fun replace_value_is_correct() {
        val data = "https://www.google.com?bundle=com.a.b&push=test&sub1=tt&notId=null"
        val result = Builder.replaceParamValue(data, "notId", "a123b")
        println("result: $result")

        Assert.assertTrue(result.contains("notId=a123b"))
        Assert.assertFalse(result.contains("notId=null"))
    }
}