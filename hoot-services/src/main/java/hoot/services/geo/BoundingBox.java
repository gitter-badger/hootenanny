/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2015 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.geo;

import java.util.ArrayList;
import java.util.List;

import hoot.services.db2.CurrentNodes;
import hoot.services.geo.zindex.Box;

import org.apache.commons.math.util.MathUtils;
import org.deegree.services.wps.input.BoundingBoxInput;
import org.w3c.dom.DOMException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;

/**
 * A geographic bounding box using a WGS84 coordinate system
 * 
 * Partially based on OSM's BoundingBox, bounding_box.rb, at 
 * https://github.com/openstreetmap/openstreetmap-website/
 */
public class BoundingBox
{
  public static final double LON_LIMIT = 180.0;
  public static final double LAT_LIMIT = 90.0;
  
  //coords in degrees
  
  private double minLon = GeoUtils.DEFAULT_COORD_VALUE;
  public double getMinLon() { return minLon; }
  
  private double minLat = GeoUtils.DEFAULT_COORD_VALUE;
  public double getMinLat() { return minLat; }
  
  private double maxLon = GeoUtils.DEFAULT_COORD_VALUE;
  public double getMaxLon() { return maxLon; }
  
  private double maxLat = GeoUtils.DEFAULT_COORD_VALUE;
  public double getMaxLat() { return maxLat; }
  
  @Override
  public String toString()
  {
    return 
      "minLon: " + minLon + ", minLat: " + minLat + ", maxLon: " + maxLon + ", maxLat: " + maxLat;
  }
  
  /**
   * Returns XML that can be as bounding box input for a WPS request
   * 
   * @return
   */
  public String toWpsXml()
  {
    return 
      "<wps:BoundingBoxData>" +
        "<ows:LowerCorner>" + minLon + " " + minLat + "</ows:LowerCorner>" +
        "<ows:UpperCorner>" + maxLon + " " + maxLat + "</ows:UpperCorner>" +
      "</wps:BoundingBoxData>";
  }
  
  private static final double BOUNDS_ERROR = 0.0000001;
  
  @Override
  public boolean equals(Object obj)
  {
    BoundingBox objBounds = (BoundingBox)obj;
    //allowing for a small amount of error here
    return
      MathUtils.equals(this.maxLat, objBounds.getMaxLat(), BOUNDS_ERROR) &&
      MathUtils.equals(this.maxLon, objBounds.getMaxLon(), BOUNDS_ERROR) &&
      MathUtils.equals(this.minLat, objBounds.getMinLat(), BOUNDS_ERROR) &&
      MathUtils.equals(this.minLon, objBounds.getMinLon(), BOUNDS_ERROR);
  }
  
  public BoundingBox()
  {
    
  }
  
  public BoundingBox(final double minLon, final double minLat, final double maxLon, 
    final double maxLat) throws Exception
  { 
    validateAndSetCoords(minLon, minLat, maxLon, maxLat);
  }
  
  public BoundingBox(final BoundingBox bounds)
  {
    this.maxLat = bounds.getMaxLat();
    this.maxLon = bounds.getMaxLon();
    this.minLat = bounds.getMinLat();
    this.minLon = bounds.getMinLon();
  }
  
  //bbox = minlon,minlat,maxlon,maxlat
  public BoundingBox(final String bbox) throws NumberFormatException, Exception
  {
    String[] bboxParts = bbox.split(",");
    validateAndSetCoords(
      Double.parseDouble(bboxParts[0]), 
      Double.parseDouble(bboxParts[1]), 
      Double.parseDouble(bboxParts[2]), 
      Double.parseDouble(bboxParts[3]));
  }
  
  public BoundingBox(final BoundingBoxInput wpsInput) throws Exception
  {
    //ignore CRS here for now and always assuming 4326
    validateAndSetCoords(
      wpsInput.getLower()[0], 
      wpsInput.getLower()[1], 
      wpsInput.getUpper()[0],
      wpsInput.getUpper()[1]);
  }
  
  /**
   * Constructs a bounding box from a list of coordinates
   * 
   * @param coords a list of coordinates to create the bounding box from
   */
  public BoundingBox(final List<Coordinates> coords)
  {
    double minLon = BoundingBox.LON_LIMIT + 1;
    double minLat = BoundingBox.LAT_LIMIT + 1;
    double maxLon = (-1 * BoundingBox.LON_LIMIT) - 1;
    double maxLat = (-1 * BoundingBox.LAT_LIMIT) - 1;
    
    for (Coordinates coord : coords)
    {
      if (coord.lat < minLat)
      {
        minLat = coord.lat;
      }
      if (coord.lat > maxLat)
      {
        maxLat = coord.lat;
      }
      if (coord.lon < minLon)
      {
        minLon = coord.lon;
      }
      if (coord.lon > maxLon)
      {
        maxLon = coord.lon;
      }
    }
    
    this.minLon = minLon;
    this.minLat = minLat;
    this.maxLon = maxLon;
    this.maxLat = maxLat;
  }
  
  /**
   * Creates a bounding box that contains all the specified nodes
   * 
   * @param nodes a list of nodes the bounding box should contain
   * @return a bounding box containing all of the input nodes
   * @throws Exception
   */
  public BoundingBox(final ArrayList<CurrentNodes> nodes)
  {
    double minLon = BoundingBox.LON_LIMIT + 1;
    double minLat = BoundingBox.LAT_LIMIT + 1;
    double maxLon = (-1 * BoundingBox.LON_LIMIT) - 1;
    double maxLat = (-1 * BoundingBox.LAT_LIMIT) - 1;
    
    for (CurrentNodes node : nodes)
    {
      final double lat = node.getLatitude();
      final double lon = node.getLongitude();
      if (lat < minLat)
      {
        minLat = lat;
      }
      if (lat > maxLat)
      {
        maxLat = lat;
      }
      if (lon < minLon)
      {
        minLon = lon;
      }
      if (lon > maxLon)
      {
        maxLon = lon;
      }
    }
    
    this.minLon = minLon;
    this.minLat = minLat;
    this.maxLon = maxLon;
    this.maxLat = maxLat;
  }
  
  private void validateAndSetCoords(final double minLon, final double minLat, final double maxLon, 
    final double maxLat) throws Exception
  {
    if (minLon > maxLon)
    {
      throw new Exception(
        "The minimum longitude must be less than or equal to the maximum longitude.");
    }
    if (minLat > maxLat)
    {
      throw new Exception(
        "The minimum latitude must be less than or equal to the maximum latitude.");
    }
    
    if (minLon < (-1 * LON_LIMIT) || minLon > LON_LIMIT)
    {
      throw new Exception("Invalid coordinate.  Invalid minimum longitude value: " + minLon);
    }
    this.minLon = minLon;
    
    if (minLat < (-1 * LAT_LIMIT) || minLat > LAT_LIMIT)
    {
      throw new Exception("Invalid coordinate.  Invalid minimum latitude value: " + minLat);
    }
    this.minLat = minLat;
    
    if (maxLon < (-1 * LON_LIMIT) || maxLon > LON_LIMIT)
    {
      throw new Exception("Invalid maximum longitude value: " + maxLon);
    }
    this.maxLon = maxLon;
    
    if (maxLat < (-1 * LAT_LIMIT) || maxLat > LAT_LIMIT)
    {
      throw new Exception("Invalid maximum latitude value: " + maxLat);
    }
    this.maxLat = maxLat;
  }
  
  /**
   * Converts the bounds into a format the service methods can understand
   * 
   * @return a bounds string
   */
  public String toServicesString() { return minLon + "," + minLat + "," + maxLon + "," + maxLat; }
  
  /**
   * Creates a bounding box from an OSM XML bounds node
   * 
   * @param xmlBounds OSM XML bounds node
   * @return a bounding box
   * @throws Exception 
   * @throws DOMException 
   * @throws NumberFormatException 
   */
  public static BoundingBox fromXml(final Node xml) 
    throws NumberFormatException, DOMException, Exception
  {
    NamedNodeMap nodeAttributes = xml.getAttributes();
    return
      new BoundingBox(
        Double.parseDouble(nodeAttributes.getNamedItem("minLon").getNodeValue()),
        Double.parseDouble(nodeAttributes.getNamedItem("minLat").getNodeValue()),
        Double.parseDouble(nodeAttributes.getNamedItem("maxLon").getNodeValue()),
        Double.parseDouble(nodeAttributes.getNamedItem("maxLat").getNodeValue()));
  }
  
  /**
   * Returns an XML representation of the bounds
   * 
   * @param parentXml parent XML node
   * @return an XML bounds node
   * @throws Exception
   */
  public Element toXml(final Element parentXml) throws Exception
  {
    if (!isInitialized())
    {
      throw new Exception("Bounds not initialized.");
    }
    Document doc = parentXml.getOwnerDocument();
    Element element = doc.createElement("bounds");
    element.setAttribute("minlat", String.valueOf(minLat));
    element.setAttribute("minlon", String.valueOf(minLon));
    element.setAttribute("maxlat", String.valueOf(maxLat));
    element.setAttribute("maxlon", String.valueOf(maxLon));
    return element;
  }
  
  /**
   * Determines if bounding box has been initialized.  Coordinate values are initialized with 
   * Double.MAX_VALUE.
   * 
   * @return true if the bounding box consists of only valid values; false otherwise
   */
  public boolean isInitialized()
  {
    return minLon != GeoUtils.DEFAULT_COORD_VALUE && minLat != GeoUtils.DEFAULT_COORD_VALUE && 
      maxLon != GeoUtils.DEFAULT_COORD_VALUE && maxLat != GeoUtils.DEFAULT_COORD_VALUE;
  }
  
  /**
   * Determines whether the bounding box is valid with respect to world boundaries.
   * 
   * @return true if valid; false otherwise
   */
  public boolean inWorld()
  {
    return 
      minLat >= -90 && minLat <= 90 && maxLat >= -90 && maxLat <= 90 &&
      minLon >= -180 && minLon <= 180 && maxLon >= -180 && maxLon <= 180;
  }
  
  /**
   * Adds the input bounding box to this bounding box
   * 
   * @param bounds bounds to expand by
   */
  public void add(final BoundingBox bounds)
  {
    expand(bounds, 0.0);
  }
  
  /**
   * Expands this bounding box to include the input bounding box.
   * 
   * @param bounds bounds to expand by
   * @param margin buffer amount to add to the expansion
   * @tod this is only used for changeset bounds calc, but I'm not sure this method is correct...
   * hence the reason for expand2
   */
  public void expand(final BoundingBox bounds, final double margin)
  {
    if (!isInitialized())
    {
      minLon = bounds.getMinLon();
      minLat = bounds.getMinLat();
      maxLon = bounds.getMaxLon();
      maxLat = bounds.getMaxLat();
    }
    else
    {
      if (bounds.getMinLon() < minLon)
      {
        minLon = 
          Math.max((-1 * LON_LIMIT), (bounds.getMinLon() + margin * (minLon - maxLon)));
      }
      if (bounds.getMinLat() < minLat)
      {
        minLat = 
          Math.max((-1 * LAT_LIMIT), (bounds.getMinLat() + margin * (minLat - maxLat)));
      }
      if (bounds.getMaxLon() > maxLon)
      {
        maxLon = Math.min(LON_LIMIT, (bounds.getMaxLon() + margin * (maxLon - minLon)));
      }
      if (bounds.getMaxLat() > maxLat)
      {
        maxLat = Math.min(LAT_LIMIT, (bounds.getMaxLat() + margin * (maxLat - minLat)));
      }
    }
  }
  
  /**
   * Expands or shrinks this bounding box
   * 
   * @param bounds bounds used to modify this bounding box
   * @param margin buffer; positive values expand the bounding box; negative values shrink it
   */
  public void combine(BoundingBox bounds, double margin)
  {
    if (!isInitialized())
    {
      minLon = bounds.getMinLon();
      minLat = bounds.getMinLat();
      maxLon = bounds.getMaxLon();
      maxLat = bounds.getMaxLat();
    }
    else
    {
      minLon = Math.min(bounds.getMinLon(), minLon);
      if ((minLon - (margin * -1)) >= (LON_LIMIT * -1))
      {
        minLon = minLon - (margin * -1);
      }
      minLat = Math.min(bounds.getMinLat(), minLat);
      if ((minLat - (margin * -1)) >= (LAT_LIMIT * -1))
      {
        minLat = minLat - (margin * -1);
      }
      maxLon = Math.max(bounds.getMaxLon(), maxLon);
      if ((maxLon + (margin * -1)) <= LON_LIMIT)
      {
        maxLon = maxLon + (margin * -1);
      }
      maxLat = Math.max(bounds.getMaxLat(), maxLat);
      if ((maxLat + (margin * -1)) <= LAT_LIMIT)
      {
        maxLat = maxLat + (margin * -1);
      }
    }
  }
  
  /**
   * Buffers this bounding box
   * 
   * @param margin buffer; positive values expand the bounding box; negative values shrink it
   */
  public void adjust(final double margin)
  {
    if ((minLon - (margin * -1)) >= (LON_LIMIT * -1))
    {
      minLon = minLon - (margin * -1);
    }
    if ((minLat - (margin * -1)) >= (LAT_LIMIT * -1))
    {
      minLat = minLat - (margin * -1);
    }
    if ((maxLon + (margin * -1)) <= LON_LIMIT)
    {
      maxLon = maxLon + (margin * -1);
    }
    if ((maxLat + (margin * -1)) <= LAT_LIMIT)
    {
      maxLat = maxLat + (margin * -1);
    }
  }
  
  /**
   * Adds two bounding boxes together
   * 
   * @param bounds1 first bounding box
   * @param bounds2 second bounding box
   * @param margin buffer; positive values expand the bounding box; negative values shrink it 
   * @return a bounding box
   */
  public static BoundingBox addBoundingBoxes(final BoundingBox bounds1, 
    final BoundingBox bounds2, double margin)
  {
    BoundingBox bounds = bounds1;
    bounds.combine(bounds2, margin);
    return bounds;
  }
   
  /**
   * Return the height of this bounding box
   * 
   * @return height in degrees
   */
  public double getHeight() { return maxLat - minLat; }

  /**
   * Return the width of this bounding box
   * 
   * @return width in degrees
   */
  public double getWidth() { return maxLon - minLon; }
  
  /**
   * Return the center longitude of this bounding box
   * 
   * @return center longitude coordinate
   */
  public double getCenterX() { return (minLon + maxLon) / 2.0; }

  /**
   * Return the center latitude of this bounding box
   * 
   * @return center latitude coordinate
   */
  public double getCenterY() { return (minLat + maxLat) / 2.0; }
  
  /**
   * Returns a bounding box that is the intersection this bounding box and the input bounding box
   * 
   * @param bounds the bounds to compute the intersection from
   * @return the bounding box intersection
   * @throws Exception
   */
  public BoundingBox intersection(BoundingBox bounds) throws Exception
  {
    return new BoundingBox(
      Math.max(minLon, bounds.getMinLon()), 
      Math.max(minLat, bounds.getMinLat()), 
      Math.min(maxLon, bounds.getMaxLon()),
      Math.min(maxLat, bounds.getMaxLat()));
  }
  
  /**
   * Determines whether this bounding box intersects the input bounding box coordinates
   * 
   * @param minX minimum longitude
   * @param minY minimum latitude
   * @param maxX maximum longitude
   * @param maxY maximum latitude
   * @return true; if this bounding box interesect the input bounding box coordinates; 
   * false otherwise
   * @throws Exception
   */
  public boolean intersects(final double minX, final double minY, final double maxX, 
    final double maxY) throws Exception
  {
    return intersects(new BoundingBox(minX, minY, maxX, maxY));
  }

  /**
   * Determines whether this bounding box intersects the input bounding box
   * 
   * @param bounds the bounds to compute the intersection with this bounding box from
   * @return true if this intersects the input bounding box; false otherwise
   */
  public boolean intersects(final BoundingBox bounds)
  {
    // formula:
    // x, y are center points of the rectangle...
    //
    //    if Math.abs(rectA.x - rectB.x) < (Math.abs(rectA.width + rectB.width) / 2) 
    //      && (Math.abs(rectA.y - rectB.y) < (Math.abs(rectA.height + rectB.height) / 2))
    //    then
    //        // A and B collide
    //    end if
    return 
      (Math.abs(getCenterX() - bounds.getCenterX()) < Math.abs(getWidth() + bounds.getWidth()) / 2) &&
      (Math.abs(getCenterY() - bounds.getCenterY()) < Math.abs(getHeight() + bounds.getHeight()) / 2);
  }
  
  /**
   * Returns the area of this bounding box
   * 
   * @return area in degrees
   */
  public double getArea()
  {
    if (!isInitialized())
    {
      return 0;
    }
    return (maxLon - minLon) * (maxLat - minLat);
  }
  
  /**
   * Returns a bounds that can be used by ZValues
   * 
   * @return a z-index bounds
   */
  public Box toZIndexBox()
  {
    //use y, x ordering
    return new Box(new double[] { minLat, minLon }, new double[] { maxLat, maxLon });
  }
  
  /**
   * Returns a bounding box that encompasses the entire world
   * 
   * @return a bounding box
   * @throws Exception
   */
  public static BoundingBox worldBounds() throws Exception
  {
    return new BoundingBox(-1 * LON_LIMIT, -1 * LAT_LIMIT, LON_LIMIT, LAT_LIMIT);
  }
  
  @Override
  public int hashCode()
  {
    return super.hashCode();
  }
}