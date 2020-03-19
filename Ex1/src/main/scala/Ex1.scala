import org.apache.spark.SparkContext
import org.apache.spark.SparkConf

object Ex1 {
  def main(args: Array[String]) {
   // val spark = SparkSession.builder.appName("Simple Application").getOrCreate()
    val conf = new SparkConf().setAppName("Ex1")
    val sc = new SparkContext(conf)
    val lines = sc.textFile("data.txt", 4)
    val lineLengths = lines.map(s => s.length)
    val totalLength = lineLengths.reduce((a, b) => a + b)

    println("Total Length : " + totalLength)

    lines.take(10).foreach(println)
  }
}
