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
package hoot.services.review;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import javax.ws.rs.core.Response.Status;

import org.apache.commons.lang3.StringUtils;
import org.deegree.commons.ows.exception.OWSException;
import org.deegree.services.wps.ProcessletException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.mysema.query.sql.SQLQuery;

import hoot.services.db.DbUtils;
import hoot.services.db2.ElementIdMappings;
import hoot.services.db2.QElementIdMappings;
import hoot.services.db2.QReviewItems;
import hoot.services.db2.ReviewItems;
import hoot.services.models.osm.Element;
import hoot.services.models.osm.Element.ElementType;
import hoot.services.utils.ResourceErrorHandler;

/**
 * Various utilities for conflated data reviewing
 */
public class ReviewUtils
{
  private static final Logger log = LoggerFactory.getLogger(ReviewUtils.class);

  protected static final QReviewItems reviewItemsTbl = QReviewItems.reviewItems;
  protected static final QElementIdMappings elementIdMappings = QElementIdMappings.elementIdMappings;

  /**
   * toString() implementation for the ElementIdMappings
   *
   * @param idMappings element ID mappings
   * @return a readable string
   */
  public static String elementIdMappingsDbPojoToString(final ElementIdMappings idMappings)
  {
    return "\nElement ID mapping:\nUUID: " + idMappings.getElementId().toString() +
      "\nElement ID: " + idMappings.getOsmElementId() + "\nElement type: " +
      idMappings.getOsmElementType().toString();
  }

  /**
   * toString() implementation for the ReviewItems
   *
   * @param item review item
   * @return a readable string
   */
  public static String reviewItemDbPojoToString(final ReviewItems item)
  {
    return "\nReview item:\nUUID: " + item.getReviewableItemId().toString() + "\nReview score: " +
      item.getReviewScore() + "\nReview status: " + item.getReviewStatus().toString();
  }

  /**
   * Returns the IDs of all items that are currently marked as being reviewable
   *
   * @param mapId ID of the map owning the reviewable items
   * @param conn JDBC Connection
   * @return reviewable element IDs grouped by element type
   */
  public static Map<ElementType, Set<Long>> getReviewableElementIds(final long mapId,
    Connection conn)
  {
    Map<ElementType, Set<Long>> reviewableElementIdsByType = new HashMap<ElementType, Set<Long>>();
    for (ElementType elementType : ElementType.values())
    {
      if (!elementType.equals(ElementType.Changeset))
      {
      	SQLQuery query = new SQLQuery(conn, DbUtils.getConfiguration(mapId));
        final Set<Long> reviewableElementIds =
          new HashSet<Long>(

            	query	.from(reviewItemsTbl)
            	.join(elementIdMappings)
            	.on(
            			reviewItemsTbl.reviewableItemId.eq(elementIdMappings.elementId)
            			.and(reviewItemsTbl.mapId.eq(mapId))
            			.and(elementIdMappings.mapId.eq(mapId))
            			.and(
            					elementIdMappings.osmElementType.eq(Element.elementEnumForElementType(elementType))
            					)
            			)
          		.where(
          				reviewItemsTbl.reviewStatus.eq(DbUtils.review_status_enum.unreviewed)
          		)
          		.list(elementIdMappings.osmElementId)

         );
        reviewableElementIdsByType.put(elementType, reviewableElementIds);
      }
    }
    return reviewableElementIdsByType;
  }

  /**
   * Converts a review status string to a review status enum value
   *
   * @param reviewStatusStr a review status string
   * @return review status enum value
   */
  public static DbUtils.review_status_enum reviewStatusFromString(final String reviewStatusStr)
  {
    if (!StringUtils.isEmpty(reviewStatusStr))
    {
      if (reviewStatusStr.toLowerCase().equals("reviewed"))
      {
        return DbUtils.review_status_enum.reviewed;
      }
      else if (reviewStatusStr.toLowerCase().equals("unreviewed"))
      {
        return DbUtils.review_status_enum.unreviewed;
      }
    }
    return null;
  }

  /**
   * Returns a unique element ID for a given map/OSM element ID/OSM element type combo
   *
   * @param mapId ID of the map owning the element
   * @param osmElementId the element's OSM ID
   * @param elementType the element's type
   * @param conn JDBC Connection
   * @return the element's unique ID
   */
  public static String getUniqueIdForElement(final long mapId, final long osmElementId,
    final ElementType elementType, Connection conn)
  {

  	SQLQuery query = new SQLQuery(conn, DbUtils.getConfiguration(mapId));

    return
  	query	.from(elementIdMappings)
		.where(
				elementIdMappings.mapId.eq(mapId)
				.and(elementIdMappings.osmElementType.eq(Element.elementEnumForElementType(elementType)))
				.and(elementIdMappings.osmElementId.eq(osmElementId))
		)
		.singleResult(elementIdMappings.elementId);

  }

  /**
   * Returns the element id for a unique item ID
   *
   * @param itemId item unique ID
   * @param map ID ID of the map the element belongs to
   * @param conn JDBC Connection
   * @return an element ID
   */
  public static long getElementId(final String itemId, final long mapId, Connection conn)
  {
  	SQLQuery query = new SQLQuery(conn, DbUtils.getConfiguration(mapId));

  	return
  	query	.from(elementIdMappings)
		.where(
				elementIdMappings.elementId.eq(itemId)
				.and(elementIdMappings.mapId.eq(mapId))
		)
		.singleResult(elementIdMappings.osmElementId);
  }

  /**
   * Returns the element type for a unique item ID
   *
   * @param itemId item unique ID
   * @param map ID ID of the map the element belongs to
   * @param conn JDBC Connection
   * @return an element type
   */
  public static ElementType getElementType(final String itemId, final long mapId,
    Connection conn)
  {
  	SQLQuery query = new SQLQuery(conn, DbUtils.getConfiguration(mapId));

  	return
  			Element.elementTypeForElementEnum(
			  	query	.from(elementIdMappings)
					.where(
							elementIdMappings.elementId.eq(itemId)
							.and(elementIdMappings.mapId.eq(mapId))
					)
					.singleResult(elementIdMappings.osmElementType)
		);
  }

  /**
   * Handles all thrown exceptions from both WPS and non-WPS review services
   *
   * @param e a thrown exception
   * @param errorMessageStart text to prepend to the error message
   * @param throwWpsError if true; throws a deegree ProcessletException; otherwise a
   * Jersey WebApplicationException is thrown
   * @throws Exception
   */
  public static void handleError(final Exception e, final String errorMessageStart,
    final boolean throwWpsError) throws Exception
  {
    Status status = null;
    if (!StringUtils.isEmpty(e.getMessage()))
    {
      if (e.getMessage().contains("Invalid input parameter") ||
          e.getMessage().contains("Invalid reviewed item") ||
          e.getMessage().contains("Error parsing unique ID tag") ||
          e.getMessage().contains("empty String") ||
          e.getMessage().contains("Invalid coordinate"))
      {
        status = Status.BAD_REQUEST;
      }
      else if (e.getMessage().contains("record exists") ||
               e.getMessage().contains("records exist") ||
               e.getMessage().contains("to be updated does not exist"))
      {
        status = Status.NOT_FOUND;
      }
      else if (e.getMessage().contains("Invalid version") ||
               e.getMessage().contains("Invalid changeset ID") ||
               e.getMessage().contains("references itself") ||
               e.getMessage().contains("Changeset maximum element threshold exceeded") ||
               e.getMessage().contains("was closed at") ||
               e.getMessage().contains("has become out of sync"))
      {
        status = Status.CONFLICT;
      }
      else if (e.getMessage().contains("has not had review data prepared for it") ||
               e.getMessage().contains(
                 "prepare job must complete before attempting to retrieve items for review") ||
               e.getMessage().contains("exist specified for") ||
               e.getMessage().contains("exist for") ||
               e.getMessage().contains("is still used by") ||
               e.getMessage().contains("Invalid review prepare job status")/*||
               //TODO: is this one valid?
               e.getMessage().contains("No records available for review")*/)
      {
        status = Status.PRECONDITION_FAILED;
      }
    }
    if (status == null)
    {
      status = Status.INTERNAL_SERVER_ERROR;
    }
    String message = "Error " + errorMessageStart + ": ";
    if (e.getMessage() != null && e.getMessage().contains("empty String"))
    {
      //added for giving a better error message when passing invalid params to jersey
      message += "Invalid input parameter";
    }
    else
    {
      message += e.getMessage();
    }
    if (e instanceof SQLException)
    {
    	SQLException sqlException = (SQLException)e;
    	if (sqlException.getNextException() != null)
    	{
    		message += "  " + sqlException.getNextException().getMessage();
    	}
    }
    if (e.getCause() instanceof SQLException)
    {
    	SQLException sqlException = (SQLException)e.getCause();
    	if (sqlException.getNextException() != null)
    	{
    		message += "  " + sqlException.getNextException().getMessage();
    	}
    }
    final String exceptionCode = status.getStatusCode() + ": " + status.getReasonPhrase();
    log.error(exceptionCode + " " + message);
    if (throwWpsError)
    {
      throw new ProcessletException(new OWSException(message, exceptionCode));
    }
    else
    {
      ResourceErrorHandler.handleError(message, status, log);
    }
  }
  
  /**
   * Creates a record to represent the mapping of an element unique ID to an osm element ID
   * 
   * @param uniqueElementId element unique ID
   * @param osmElementId element OSM ID
   * @param elementType element's type
   * @param mapId ID of map owning element
   * @return an element ID mapping database record
   */
  public static ElementIdMappings createElementIdMappingRecord(final String uniqueElementId, 
		final long osmElementId, final ElementType elementType, final long mapId)
	{
		ElementIdMappings elementIdMappingRecord = new ElementIdMappings();
		log.debug("uniqueElementId: " + uniqueElementId);
		elementIdMappingRecord.setElementId(uniqueElementId);
		log.debug("mapId: " + mapId);
		elementIdMappingRecord.setMapId(mapId);
		log.debug("osmElementId: " + osmElementId);
		elementIdMappingRecord.setOsmElementId(osmElementId);
		log.debug("elementType: " + elementType.toString());
		elementIdMappingRecord.setOsmElementType(Element
		    .elementEnumForElementType(elementType));
		return elementIdMappingRecord;
	}
  
  /**
   * Creates a record to represent the mapping of an element unique ID to an osm element ID
   * 
   * @param reviewableItemId the unique ID of the item being reviewed
   * @param reviewScore the importance score of the associated review
   * @param reviewAgainstItemId the unique ID of the item the reviewable item is being reviewed
   * against
   * @param mapId ID of map owning the review elements
   * @return a review item database record
   */
  public static ReviewItems createReviewItemRecord(final String reviewableItemId,
	  final double reviewScore, final String reviewAgainstItemId, final long mapId)
	{
  	log.debug("Creating review item record: ");
		ReviewItems reviewItemRecord = new ReviewItems();
		log.debug("mapId: " + mapId);
		reviewItemRecord.setMapId(mapId);
		log.debug("reviewableItemId: " + reviewableItemId);
		reviewItemRecord.setReviewableItemId(reviewableItemId);
		reviewItemRecord.setReviewScore(reviewScore);
		if (reviewAgainstItemId != null)
		{
			log.debug("reviewAgainstItemId: " + reviewAgainstItemId);
			reviewItemRecord.setReviewAgainstItemId(reviewAgainstItemId);
		}
		reviewItemRecord.setReviewStatus(DbUtils.review_status_enum.unreviewed);
		return reviewItemRecord;
	}
}
