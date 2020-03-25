import org.apache.spark.graphx.GraphLoader
import org.apache.spark.sql.SparkSession

object PageRank {
  def main(args: Array[String]): Unit = {

    var fileName = ""
    var appNameGiven = ""
    if(args.length == 2) {
	    fileName = args(0)
        appNameGiven = args(1)
    }
    else {
	    println("Usage : " + "spark-submit --class '<class_name>' --master <master_url> <target_jar> <file_name> <appName>")
	    return
    }
    println("File Name : " + fileName)
    println("App Name : " + appNameGiven)
    val spark = SparkSession
      .builder
      .appName(appNameGiven)
      .getOrCreate()
    val sc = spark.sparkContext
    // Load the edges as a graph
    //val graph = GraphLoader.edgeListFile(sc, "data/graphx/followers.txt")
    //val graph = GraphLoader.edgeListFile(sc, "hdfs://10.10.1.2:9000/cs740/graphx/data/followers.txt")
    val graph = GraphLoader.edgeListFile(sc, fileName)
    // Run PageRank
    val ranks = graph.pageRank(0.0001).vertices
    // Print the result
    println(ranks.collect().mkString("\n"))
    spark.stop()
  }
}
