import org.apache.spark.graphx.GraphLoader
import org.apache.spark.sql.SparkSession

object PageRank {
  def main(args: Array[String]): Unit = {
    val spark = SparkSession
      .builder
      .appName(s"${this.getClass.getSimpleName}")
      .getOrCreate()
    val sc = spark.sparkContext

    var fileName = ""
    if(args.length == 1) {
	fileName = args(0)
    }
    else {
	println("Usage : " + "spark-submit --class '<class_name>' --master <master_url> <target_jar> <file_name>")
	spark.stop()
	return
    }
    println("File Name : " + fileName)
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
