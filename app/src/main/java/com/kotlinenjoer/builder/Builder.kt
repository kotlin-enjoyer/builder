package com.kotlinenjoer.builder

object Builder {
    init {
        System.loadLibrary("builder")
    }

    external fun buildQuery(domain: String, params: Map<String, String>): String

    external fun replaceParamValue(query: String, paramName: String, paramValue: String): String

    external fun getParamValue(query: String, paramName: String): String
}